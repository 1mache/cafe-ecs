<div dir="rtl">

# RenderSortBench — בנצ'מרק לשלוש גרסאות המיון של הרנדר

הקובץ `app/render_sort_bench.cpp` הוא נקודת כניסה נפרדת (לא חלק מהמשחק) שמודדת כמה מיקרו־שניות לפריים עולה **סידור ציור הספרייטים לפי שכבות** בשלוש הגרסאות שהיו לנו בהיסטוריה: הגרסה הישנה, גרסת ה־branch של ה־sorted render, והגרסה הנוכחית. הוא מודד רק את עבודת הסידור — לולאת הציור עצמה (GPU) זהה בכולן, אז ההשוואה מבודדת בדיוק את מה שהשתנה.

## שלוש הגרסאות

**old** — מה שהיה לפני ה־sorted render: כל פריים אוספים את כל הישויות עם `Drawable+Transform` ל־`std::vector` חדש וממיינים מאפס. הקצאת זיכרון + מיון מלא כל פריים, גם כשכלום לא השתנה.

<div dir="ltr">

```cpp
// --- OLD (51e2db1): fresh vector + full sort, every frame ---
long long orderOld()
{
    std::vector<Entity> drawables{};
    for (auto e = Entity::first(); !e.eof(); e.next())
        if (e.test(drawMask))
            drawables.push_back(e);

    std::sort(drawables.begin(), drawables.end(), ...by renderLayer...);
    ...
}
```

</div>

**new** ו־**fix** חולקות את אותו מבנה — cache קבוע (`sorted`), מפת חברוּת לפי id‏ (`inSorted`) ו־dirty bit. כל פריים עושים סריקה אחת: ישות שקיבלה `Drawable` נכנסת ל־cache, ישות שנהרסה יוצאת ממנו, ורק שינוי כזה מדליק את ה־dirty. ההבדל ביניהן הוא **איך ממיינים כשה־dirty דולק**:

<div dir="ltr">

```cpp
struct SortedCache
{
    bool stepSort = false; // true = new (branch tip), false = fix (current)
    ...
    if (stepSort)
    {
        // Branch tip: one bubble pass; clear dirty only once fully sorted.
        bool isSorted = true;
        for (int i = 0; i < sorted.size() - 1; ++i)
            if (sorted[i].get<Drawable>().renderLayer >
                sorted[i + 1].get<Drawable>().renderLayer)
            { isSorted = false; std::swap(sorted[i], sorted[i + 1]); }
        if (isSorted) dirty = false;
    }
    else
    {
        std::stable_sort(...by renderLayer...);
        dirty = false;
    }
```

</div>

כלומר: **new** מנסה לתקן את הסדר בצעד bubble אחד לפריים, **fix** (מה שרץ היום ב־`drawSystem`) עושה מיון מלא אבל רק בפריימים שבהם משהו באמת השתנה.

## שתי הסצנות

**static** — ‏400 ישויות עם שכבות אקראיות, שום דבר לא משתנה. זה המקרה הנפוץ במשחק.

**churn** — אותו בסיס + 120 "טיפות" חיות, וכל פריים נהרסות 30 ונוצרות 30 חדשות בשכבות מעורבות — מדמה מזיגה מהירה עם חלקיקים וטקסטים שנולדים ומתים ביחד. זה ה־worst case של ה־cache.

<div dir="ltr">

```cpp
constexpr int BASE_ENTITIES   = 400;  // static scene size (~real game scale)
constexpr int CHURN_DROPS     = 120;  // live "liquid drops" during churn
constexpr int CHURN_PER_FRAME = 30;   // drops destroyed + spawned each frame
constexpr int TIMED_FRAMES    = 2000;
constexpr unsigned SEED       = 42;   // fixed: both impls see identical scenes
```

</div>

\*הערה: בלולאת ה־churn קודם יוצרים את הישות החדשה ורק אז הורסים את הישנה. אם הורסים ואז יוצרים באותו פריים, bagel ממחזר את אותו id מיד — וה־cache בכלל לא רואה שינוי (החברוּת לפי id לא השתנתה). זו בעיה סמויה שקיימת גם ב־`drawSystem` האמיתי, אבל היא לא מזיקה שם כי ישויות ממוחזרות הן תמיד טיפות באותה שכבה.

## ה־checksum — הוכחת נכונות, לא רק מהירות

כל גרסה מחזירה סכום משוקלל לפי מיקום: `sum += (i+1) * renderLayer`. אם רצף השכבות שיוצא זהה — הסכום זהה. ככה רואים לא רק מי מהיר יותר, אלא גם **מי מצייר בסדר הנכון**. כשה־checksum של גרסה שונה מזה של old, הבנצ'מרק מדפיס `WRONG ORDER`.

## איך מריצים

מחליפים את השורה הראשונה ב־`app/CMakeLists.txt` (זו הקונבנציה של הפרויקט לנקודות כניסה לבדיקות), בונים ומריצים:

<div dir="ltr">

```cmake
set(APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/render_sort_bench.cpp")
```

```bash
cmake --build cmake-build-debug && ./cmake-build-debug/app/main
```

</div>

לא לשכוח להחזיר את השורה ל־`game_main.cpp` בסיום.

## התוצאות (debug build)

<div dir="ltr">

```text
impl                         scene    avg us/frame max us/frame         checksum
old: gather+sort each frame  static          51.39        75.92           704645
new: cache + step sort       static           7.81        19.92           704645
fix: cache + full sort       static           7.33        27.00           704645

old: gather+sort each frame  churn           58.74       126.46          1183234
new: cache + step sort       churn           18.85        41.12          1083868  <- WRONG ORDER
fix: cache + full sort       churn           44.98        66.46          1183234
```

</div>

הסיפור בשורה אחת: old נכון אבל איטי (~51µs תמיד); new מהיר פי 6 אבל תחת churn מעורב־שכבות ה־bubble pass לא מדביק את הקצב והסדר פשוט שגוי — זה ההבהוב שראינו במשחק; fix שומר על המהירות במקרה הנפוץ (~7µs) ונשאר נכון גם ב־churn‏ (45µs — עדיין מהיר מ־old, כי אין איסוף מחדש ואין הקצאה).

\*הערה: המספרים הם מ־debug build. היחס בין הגרסאות הוא מה שחשוב; ב־Release הפערים המוחלטים יתכווצו אבל הסיפור נשאר זהה.

\*הערה: הקובץ הוא test entry point — לפי ה־CLAUDE.md של הפרויקט הוא לא אמור להתמזג ל־main. להשאיר אותו בענף לצורך ההדגמה ולהסיר לפני מיזוג.

</div>

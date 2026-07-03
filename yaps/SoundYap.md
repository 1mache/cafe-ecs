


<div dir="rtl">

# צלילים במשחק

הוספנו תשתית סאונד. המבנה מקביל ל-`AssetManager` ו-`PhysicsContext`: אובייקט ש-`Scene` מחזיק, מאותחל פעם אחת, ומועבר by reference למערכות שצריכות להשמיע צליל.

## איך זה עובד

`AudioContext` מחזיק את מכשיר האודיו ו-8 "voices" שה-SDL ממקסס יחד אוטומטית. הוא טוען קבצי WAV מהתיקייה `res/sounds/` ומקצ'ר אותם לפי שם — טעינה ראשונה מהדיסק, אחר כך מהזיכרון:

<div dir="ltr">

```cpp
AudioContext& audio = getAudioContext();
audio.play("click.wav");   // res/sounds/click.wav — מנגן ומשכיח
```

</div>

`play` הוא fire-and-forget: מקצ'ר קול קצר על voice פנוי. אם כל ה-8 תפוסים, הצליל נופל. מתאים לאפקטים קצרים (לחיצות, טפטוף).

\*הערה: רק WAV. קובץ MP3 ייכשל בטעינה. אם צריך — להמיר עם `ffmpeg -i x.mp3 x.wav`.

## צליל מתמשך (לחיצה-והחזקה)

לצלילים שמתנגנים כל עוד מחזיקים כפתור (כמו מקציף החלב) יש ערוץ נפרד. `startSustained` בהתחלה, `stopSustained` בשחרור:

<div dir="ltr">

```cpp
// startOffsetSeconds מדלג לתוך הקובץ (למשל מעבר ל-fade-in איטי)
void startSustained(std::string_view filename, float startOffsetSeconds = 0.f);
void stopSustained();  // עוצר מיד
```

</div>

`startSustained` בטוח לקרוא כל פריים בזמן שהכפתור מוחזק — הוא לא מתחיל מחדש אם **אותו** צליל כבר מנגן. הערוץ זוכר איזה צליל פעיל, אז קריאה עם שם אחר מחליפה את הקודם. `stopSustained` מנקה את הערוץ ומשתיק מיד. יש **ערוץ מתמשך אחד בלבד** — שני צלילי החזקה לא יכולים להתנגן יחד.

`startOffsetSeconds` מדלג את תחילת הקובץ. משמש לקפה, שיש לו lead-in איטי — מתחילים כמה שניות פנימה.

## שמות הקבצים במקום אחד

אין לפזר שמות קבצים כמחרוזות בתוך המערכות. כל שמות ה-WAV מרוכזים ב-`SoundAssets.h` תחת `namespace cafe::sound`, וקוראים להם דרך `sound::NAME`:

<div dir="ltr">

```cpp
namespace cafe::sound
{
// WAV file names under res/sounds/, passed to AudioContext::play / startSustained.
constexpr auto COFFEE       = "coffee.wav";
constexpr auto MILK_STEAMER = "milk_steamer.wav";
} // namespace cafe::sound
```

</div>

מוסיפים צליל חדש → מוסיפים כאן שורה, ומשתמשים ב-`sound::NAME`. אף מחרוזת קובץ לא חוזרת בקוד המערכות.

## דוגמה: מזיגת קפה וחלב

צינורות הקפה והחלב הם `LiquidSpawner` עם `kind == Coffee` / `Milk`, ו-`active` נדלק כל עוד מקש `1` (קפה) או `3` (חלב) מוחזק. ב-`liquidSpawnerSystem` בודקים איזה צינור מוזג ומפעילים את הצליל המתאים. שם הקובץ מגיע מ-`sound::`, וההשהיה מקבוע מקומי:

<div dir="ltr">

```cpp
if (coffeePouring)
    audio.startSustained(sound::COFFEE, COFFEE_POUR_SOUND_DELAY); // skip the file's slow lead-in
else if (milkPouring)
    audio.startSustained(sound::MILK_STEAMER);
else
    audio.stopSustained();
```

</div>

התוצאה: מחזיקים `1` → קפה מתנגן (מדלג את ה-lead-in); מחזיקים `3` → מקציף החלב; משחררים → נעצר מיד. כי יש ערוץ אחד, אם שניהם מוחזקים ביחד — **קפה מנצח**.

## איך משתמשים

1. שים קובץ WAV ב-`res/sounds/` (הוא מועתק ליד ה-exe אוטומטית), והוסף את שמו ל-`SoundAssets.h`.
2. הוסף `AudioContext&` לחתימת המערכת (בדיוק כמו `PhysicsContext&`), והעבר `getAudioContext()` בקריאה מ-`MainGameScene`.
3. אפקט קצר → `play(sound::NAME)`. צליל שמתנגן תוך כדי החזקה → `startSustained(sound::NAME)` / `stopSustained`.

</div>
#pragma once

namespace cafe
{
/** @brief The single place entities actually die. Any system that decides an
 *  entity should be destroyed sets Destroy{} on it instead of destroying it
 *  directly; this system runs last in the frame and does the rest:
 *
 *  1. Closure — propagates Destroy to anything that references a tagged
 *     entity (ChildOf.parent, Liquid/Ice.holdingContainer), looping to a
 *     fixpoint so no id is freed while dependents are still being discovered.
 *  2. Collect  — gathers every Destroy-tagged id.
 *  3. Destroy  — destroyPhysicalEntity() on each collected id.
 *
 *  Call this after every system that can tag Destroy{}, and before drawSystem. */
void destroySystem();
} // namespace cafe

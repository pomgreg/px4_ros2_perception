# Contributing

## Commit Message Format

```
[TAG] type: short description
```

Example: `[CMD] feat: add keyboard control via /dev/tty`

---

## Tags

| Tag | Scope |
|-----|-------|
| `[PX4]` | PX4 firmware interface, messages, modes |
| `[CMD]` | Drone command & control node |
| `[PROJ]` | Project-level docs, README, config files |

> New tags will be added as the project grows (e.g. `[TRACK]` for tracking, `[SIM]` for simulation).

---

## Types

| Type | When to use |
|------|-------------|
| `feat` | New feature or capability |
| `fix` | Bug fix |
| `refactor` | Code restructure, no behavior change |
| `docs` | Comments, learning notes, non-README docs |
| `chore` | Build system, dependencies, CI |
| `test` | Adding or updating tests |

---

## Examples

```
[PX4] feat: add offboard position control
[PX4] fix: set unused TrajectorySetpoint fields to NaN
[CMD] feat: add keyboard control via /dev/tty
[CMD] refactor: replace select() with dedicated std::thread
[CMD] fix: restore terminal settings on node shutdown
[PROJ] docs: update README with launch instructions
[PROJ] chore: add CONTRIBUTING.md
```

---

## Rules

- Description in **lowercase**, no period at the end
- Keep it under **72 characters** total
- Use the **imperative tense** : `add`, `fix`, `remove` — not `added`, `fixed`, `removed`
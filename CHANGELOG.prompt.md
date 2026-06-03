# CHANGELOG generation prompt

Use this prompt to (re)generate `CHANGELOG.md` for the BespokeSynth project after
a long unreleased period — possibly with a different grouping than what's
currently checked in. The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

The instructions below tell you exactly what to query and how, so you don't
need any prior context about the project's release history.

---

## 1. Gather inputs

Do these steps in order. Don't skip; later steps depend on data from earlier
ones.

### 1a. Find the most recent released version section

Read the existing `CHANGELOG.md` and locate the topmost `## [X.Y.Z] - YYYY-MM-DD`
heading. That's the cutoff. Save the version string as `$LAST_VERSION` (e.g.
`1.3.0`) and the date as `$LAST_DATE`.

### 1b. Find the commit that shipped that version

Tags may not exist locally. Try the tag first, fall back to a message search:

```bash
git rev-parse --verify "v$LAST_VERSION" 2>/dev/null \
  || git log --all --oneline --grep="^$LAST_VERSION" | head -1 | awk '{print $1}'
```

Save the SHA as `$LAST_RELEASE`.

### 1c. Enumerate every commit since `$LAST_RELEASE`

```bash
git log $LAST_RELEASE..HEAD \
  --pretty=format:"%h|%ad|%s" --date=short --no-merges
```

Save the full list — every line gets classified in step 3.

### 1d. Tally commit volume by month and by quarter

You'll need this to choose a grouping strategy intelligently.

```bash
# Monthly
git log $LAST_RELEASE..HEAD --pretty=format:"%ad" --date=short --no-merges \
  | awk -F- '{print $1"-"$2}' | sort | uniq -c

# Quarterly
git log $LAST_RELEASE..HEAD --pretty=format:"%ad" --date=short --no-merges \
  | awk -F- '{ q=int(($2-1)/3)+1; print $1" Q"q }' | sort | uniq -c
```

If the unreleased period is shorter than ~3 months, prefer **A. Flat** (see
step 2). If many months have ≤5 commits, prefer **B. Quarterly** over
**C. Monthly**.

### 1e. Pull any existing WIP changelog draft

A contributor may have started a changelog branch on a fork. As of 2026-05,
the canonical WIP for the post-1.3.0 changelog is
`NoxyNixie/BespokeSynth:changelog-post-1.3.0`. Fetch it with:

```bash
curl -sL https://raw.githubusercontent.com/NoxyNixie/BespokeSynth/changelog-post-1.3.0/CHANGELOG.md \
  -o /tmp/wip-changelog.md
```

If the user points to a different fork/branch, swap the URL accordingly. If
none exists, skip this step.

**Treat WIP entries as already-distilled, user-facing prose.** Keep them
verbatim where possible; don't re-paraphrase. The contributor has already
done the editorial work of turning commit subjects into changelog bullets.

### 1f. Date-stamp each WIP entry

For each WIP bullet, find its originating commit (search by PR number `#NNNN`
if present, otherwise by keyword) and record the merge date so it can be
placed in the right time bucket:

```bash
# By PR number
git log $LAST_RELEASE..HEAD --grep="#1888" --pretty=format:"%h %ad %s" \
  --date=short
# By keyword
git log $LAST_RELEASE..HEAD --grep="voicemanager" -i \
  --pretty=format:"%h %ad %s" --date=short
```

---

## 2. Choose a grouping strategy

Ask the user, or pick a default based on the volume tally from step 1d:

- **A. Flat** — single `## [unreleased]` with one Added / Changed / Fixed /
  Removed list each. Standard Keep-a-Changelog. Best when the unreleased
  period is short (<3 months) or commit volume is low (<40 commits).
- **B. Quarterly subsections (default for long periods)** — `## [unreleased]`
  with `### YYYY Q# (Mon–Mon YYYY)` subheadings, each containing the four
  buckets. Reverse-chronological (newest quarter first). Empty buckets
  omitted. Best when commits span >6 months and >100 in volume.
- **C. Monthly subsections** — same as B but `### YYYY-MM`. Use only when
  commit volume is consistently >10/month; otherwise quiet months become
  noise.
- **D. Retroactive intermediate releases** (`1.3.1`, `1.3.2`, …) — only if
  the project actually plans to *tag* those points now. Don't fabricate
  releases that never shipped.

When in doubt, use **B**.

---

## 3. Classify every commit

For each line of `git log $LAST_RELEASE..HEAD`, choose one bucket:

| Commit pattern | Bucket |
|---|---|
| New module, new effect, new modulator, new controller layout, new scripting API | **Added** |
| Behaviour tweak to existing module, UI/UX adjustment, default change, perceivable performance change | **Changed** |
| `fix:` / "Fixed …" / "Resolves #NNNN" / crash repairs | **Fixed** |
| "Removed …" / `revert:` of a public-facing feature | **Removed** |
| Merge-only, CI tweak, build-system change, internal refactor with no user-visible effect | **drop** |

When the subject line is ambiguous, fetch the PR body for context:

```bash
gh pr view <PR-number> --json title,body
```

(Requires `gh` configured against `BespokeSynth/BespokeSynth`.)

---

## 4. Write the entries

Style conventions, derived from the existing CHANGELOG:

- **One bullet per user-facing change.** Several commits collapse into one
  bullet if they touch the same feature.
- **Sentence form.** "Added the `foo` module" or "`foo` now does X". Lead
  with a verb in past tense for Added/Changed/Fixed bullets.
- **No trailing period** on bullets — matches the existing 1.3.0 section.
- **Backtick all module names**: `audiosyncer`, `looperrecorder`, etc. The
  existing CHANGELOG is consistent about this.
- **Don't reference PR numbers** in bullets. The compare-link footer makes
  the PR list discoverable already.
- **Don't mention internal refactors** unless they enable a user-visible
  benefit ("refactor PlayNote() to take a structure" → drop).

For grouping B/C, sort entries within a bucket newest-first by commit date
when the order matters; otherwise group by topic.

---

## 5. Output format

Update `CHANGELOG.md` in place:

1. Insert the new `[unreleased]` section directly above the most recent
   released section (the one with version `$LAST_VERSION`).
2. Lay out subsections per the chosen grouping strategy.
3. Update the bottom-of-file compare-link footer:
   ```markdown
   [unreleased]: https://github.com/BespokeSynth/BespokeSynth/compare/v$LAST_VERSION...main
   ```
4. **Do not modify any released section** (1.3.0 and earlier). Released
   sections are immutable history.

### Example structure for grouping B (quarterly)

```markdown
## [unreleased] - ####-##-##

### 2026 Q2 (April–May 2026)

#### Added
- The `audiosyncer` module, to make it easier to visually beatmatch audio
  to an external source

#### Changed
- ctrl-shift-click on a UI control randomizes its value

#### Fixed
- Rare crash when logging from the audio thread while drawing the console

### 2026 Q1 (January–March 2026)

#### Added
- Welcome screen with recently-saved files
- The `dopplershift` module
…
```

### Example structure for grouping A (flat)

```markdown
## [unreleased] - ####-##-##

### Added
- The `audiosyncer` module …
- Welcome screen with recently-saved files
- The `dopplershift` module
…

### Changed
- ctrl-shift-click on a UI control randomizes its value
…
```

---

## 6. Sanity checks before finishing

Run all four:

1. **Conversion-rate check.**
   ```bash
   git log $LAST_RELEASE..HEAD --no-merges --oneline | wc -l
   ```
   Expect the number of changelog bullets to be ≈ 0.7–0.85 × that count.
   Significantly lower → you dropped too many. Significantly higher → you
   didn't collapse related commits into single bullets.

2. **No WIP regressions.** Every bullet from `/tmp/wip-changelog.md` (step
   1e) must still appear somewhere in the new file. Verify with:
   ```bash
   # Pick a few distinctive WIP phrases and grep:
   grep -F "voicemanager" CHANGELOG.md
   grep -F "16 bars retroactively" CHANGELOG.md
   ```

3. **No bullet appears in two buckets.** Skim the new section.

4. **For grouping B/C: every subsection's commits fall within its date
   window.** Sample 2–3 bullets per subsection, find their commits, confirm
   the date matches the heading.

5. **Heading hierarchy is `## [unreleased]` → `### YYYY Q#` → `#### Added`**
   (or the equivalent for grouping A/C).

---

## Notes on this project's specifics

- The `## [unreleased]` placeholder in past WIP work has used
  `## [unreleased] - ####-##-##` with literal hashes for the date —
  preserve that style until an actual release date is known.
- Past releases (1.0.0, 1.1.0, 1.2.0, 1.2.1, 1.3.0) are *flat* with no
  per-month/quarter subheadings. Don't retroactively re-bucket them.
- When 1.4.0 actually ships, the current decision (per the project's plan
  file) is to **keep** the per-quarter subheadings inside the `## [1.4.0]`
  section as historical record. Future Claude sessions tagging 1.4.0
  should not collapse them unless explicitly told to.
- The repo has no local tags; `git rev-parse v1.3.0` will fail. Always use
  the message-search fallback in step 1b.

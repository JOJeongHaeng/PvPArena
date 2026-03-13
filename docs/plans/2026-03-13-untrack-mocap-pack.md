# Untrack Mocap Pack Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Stop tracking `Content/MCO_Mocap_Basics` in Git while keeping the local files on disk.

**Architecture:** Make the change only in Git metadata and ignore rules. Add the folder to `.gitignore`, remove it from the Git index with `git rm --cached -r`, then verify the files still exist locally and only the intended deletion diff is staged.

**Tech Stack:** Git, Unreal Engine content assets, `.gitignore`

---

### Task 1: Ignore the mocap pack folder

**Files:**
- Modify: `.gitignore`

**Step 1: Add the ignore rule**

Add this line to `.gitignore` if it is not already present:

```gitignore
Content/MCO_Mocap_Basics/
```

**Step 2: Review the ignore rule**

Run:

```bash
rg -n "Content/MCO_Mocap_Basics/" .gitignore
```

Expected: the new ignore entry appears exactly once.

### Task 2: Remove the folder from Git tracking without deleting local files

**Files:**
- Git index only: `Content/MCO_Mocap_Basics/**`

**Step 1: Remove the folder from the Git index**

Run:

```bash
git rm --cached -r Content/MCO_Mocap_Basics
```

Expected: Git reports staged deletions for the tracked mocap pack assets.

**Step 2: Verify the local files still exist**

Run:

```bash
find Content/MCO_Mocap_Basics -maxdepth 2 -type f | sed -n '1,20p'
```

Expected: files are still present on disk even though Git is no longer tracking them.

### Task 3: Verify scope and commit

**Files:**
- Modify: `.gitignore`
- Remove from tracking: `Content/MCO_Mocap_Basics/**`

**Step 1: Verify branch status**

Run:

```bash
git status --short
```

Expected: `.gitignore` is modified and `Content/MCO_Mocap_Basics/**` appears as deleted from Git tracking.

**Step 2: Review the diff summary**

Run:

```bash
git diff --stat --cached
```

Expected: only `.gitignore` and `Content/MCO_Mocap_Basics/**` are included.

**Step 3: Commit**

Run:

```bash
git add .gitignore
git commit -m "chore: untrack mocap content pack"
```

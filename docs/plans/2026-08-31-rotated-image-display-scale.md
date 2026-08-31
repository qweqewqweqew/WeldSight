# Rotated Image Display Scale Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Make `UI/RotatedImageScale` visibly control the size of rotated E16 result images without affecting algorithm input, archived images, or ordinary result images.

**Architecture:** Render the result image and annotations at the existing standard display size first. For a rotated E16 landscape result, scale that rendered image by the configured factor and center it on a black canvas of the original standard size, so the existing fit-to-window widget preserves the requested visible margin.

**Tech Stack:** Qt 5 `QImage`/`QPainter`, OpenCV display conversion, qmake/MSVC Release build.

---

### Task 1: Fix rotated-image display composition

**Files:**
- Modify: `CapSenseTri/capsensetriform.cpp:455-479`

**Step 1:** Keep the base result rendering at a 640-pixel long side for both ordinary and rotated images.

**Step 2:** Apply `RotatedImageScale` only when the current product uses special rotation and the result is landscape.

**Step 3:** Center the scaled result on a black canvas matching the base rendered image size.

**Step 4:** Keep ordinary images unchanged and retain the existing `0.1` to `1.0` configuration validation.

### Task 2: Verify and commit

**Files:**
- Verify: `CapSenseTri/capsensetriform.cpp`
- Verify: `configs/app_config.ini`

**Step 1:** Run the Release build with MSVC 2019 and require a successful link.

**Step 2:** Inspect the diff to confirm no algorithm, archive, threshold, or timeout behavior changed.

**Step 3:** Commit only the implementation plan and display fix; preserve all unrelated working-tree changes.

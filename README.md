# CourseCLI 🎓

> A lightweight, modern C++ command-line utility and interactive TUI engine designed for computer science students to track, prioritize, and conquer academic workloads.
---

## ⚡ Overview

**CourseCLI** bridges the gap between lightning-fast single-line data entry and a visual terminal dashboard. Built specifically for high-stress academic schedules, it calculates dynamic **Urgency Scores** using deadline proximity, assignment weights, and estimated effort—helping you focus on what actually matters *right now*.

---

## ✨ Features

- **⚡ Instant CLI Entry:** Add assignments, complete tasks, or check upcoming deadlines in seconds without leaving your flow.
- **🖥️ Interactive TUI Dashboard:** Rich visual workspace powered by [FTXUI](https://github.com/ArthurSonzogni/FTXUI) featuring course filtering, progress indicators, and keyboard shortcuts.
- **🧠 Intelligent Urgency Engine:** Automatically sorts tasks using a custom prioritization formula rather than static due-date ordering:
  $$\text{Urgency Score} = \frac{\text{Assignment Weight (\%)}}{\text{Hours Remaining} + 1}$$
- **⚠️ Workload Crunch Detection:** Flags high-stress clusters when multiple heavy assignments fall within a 48–72 hour window.
- **💾 Local Persistence:** Human-readable JSON data storage (`~/.config/course-cli/data.json`) with zero external server dependencies.

---

## 🚀 Quick Start & Usage

### 1. Interactive TUI Mode
Launch the full terminal interface by running the command with no arguments:

```bash
course-cli
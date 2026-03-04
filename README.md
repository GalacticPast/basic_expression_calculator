# basic_expression_calculator

A simple math parser that handles complex strings like `1 + 2 * 3 / 2 + 5` instead of just one operation at a time.

##  Features
* **Order of Operations:** Correctly handles PEMDAS/BODMAS.
* **Parentheses:** Supports nested logic like `(1 + 2) * 3`.
* **Decimals:** Supports 32-bit floating-point numbers (e.g., `10.5 / 2`).
* **Error Handling:** Clearer feedback for syntax mistakes.

##  How it works
1. **Lexer:** Breaks your string into tokens (numbers and symbols).
2. **Parser:** Builds a tree based on math rules.
3. **Evaluator:** Runs through the tree to give you the answer.

##  Setup & Usage
1. **Clone:** `git clone https://github.com/GalacticPast/basic_expression_calculator.git`
2. **Build:**
* **Linux:** `./build.sh`
* **Windows:** `./build.bat`


##  Roadmap / TODO
- [x] Add parentheses support.
- [x] Better error reporting.
- [x] Basic 32-bit float support.
- [ ] **Fix:** Support for very long floats (e.g., `0.22234234123412341243`) to prevent overflow.
- [ ] **New Features:** Add `^` (power), `%` (mod), `!` (factorial), and trig functions.
- [ ] **Visualize:** Create a way to see the calculation tree.


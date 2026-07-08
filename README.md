CS204 Course Project: Branch Prediction and Branch Pattern Analysis Dashboard

Overview: 

- We developed a branch prediction analysis framework using Intel PIN to study and compare modern branch prediction techniques. 
- The project implements 1-bit, 2-bit, Hybrid and TAGE predictors by executing them on multiple workloads.
- The prediction behaviour is visualised through an interactive dashboard.

Project Structure:

CS204_PROJECT


 `dashboard` 
 <br>
  `MyBranch` 
 <br>
   `pin_kit` 
 <br>
    `source` 
 <br>
     `tools` 

Predictors Implemented:

MyBranch.cpp contains 
- 1-bit predictor
- 2-bit saturating counter predictor
- Hybrid selection mechanism
- Branch-level misprediction tracking

MyTage.cpp contains 
- Global History Register (GHR)
- Tagged prediction tables
- Provider-based prediction selection
- Usefulness counters and decay tracking

History lengths used 
- T1 → 8-bit history
- T2 → 16-bit history


Workloads / Test Cases:

- basic1.cpp State - machine branch patterns 
- bubble2.cpp      - Bubble sort and nested loops 
- matrix3.cpp      - Matrix multiplication workload 
- bfs4.cpp         - Graph traversal using BFS 
- logs5.cpp        - Dijkstra / shortest path workload 
- test1.cpp        - Mixed branch instruction patterns 
- test2.cpp        - Loops, switch statements and indirect calls 



Build Instructions:

Compile PIN Tools -

`make obj-intel64/MyBranch.so TARGET=intel64`
<br>
`make obj-intel64/MyTage.so TARGET=intel64`

Compile Test Cases -

Example:
`g++ basic1.cpp -o basic1`
<br>
`g++ bubble2.cpp -o bubble2`

Execution of the Predictors - 

Example:
`$PIN_ROOT/pin -t obj-intel64/MyBranch.so -o test1 -- ./basic1`
<br>
`$PIN_ROOT/pin -t obj-intel64/MyTage.so -o test1 -- ./basic1`


Dashboard:

Generated JSON files are stored inside dashboard/results/

The dashboard visualises 
- predictor accuracy comparison
- branch-level mispredictions
- difficult branch patterns
- 1-bit vs 2-bit vs Hybrid vs TAGE comparison

Tech Stack used 
- HTML
- CSS
- JavaScript
- Chart.js




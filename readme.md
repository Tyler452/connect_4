The Connect 4 code provides a 7x6 grid with animated piece drops and menu buttons to start either a 2‑player game or play vs AI, and it serializes the board as a row‑major string for simulation.
The AI uses depth‑limited negamax with alpha‑beta pruning plus immediate win/block checks and a 4‑cell window to pick moves. 
'This AI tecnique was inspired by the conversation I had with the proffessor after my AI was playing in a not smart way as it would block sometimes and not all the time. It took alot of time to look up how it might work and then implement it'

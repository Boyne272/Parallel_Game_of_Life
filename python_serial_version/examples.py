#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Some examples of using the automata module functions,
as well as timing code for these functions.

Created on Wed Oct 24 15:36:09 2018
@author: Richard Boyne rmb115@ic.ac.uk
"""

import numpy as np
import automata as au
import time as tm


options = []  # what do you want to do
"""
1 = time the sandpile solution
2 = time game of life
3 = animate several gliders in 2d periodic
4 = animate a generator in 2d periodic
5 = animate a set of random initial condtions in 2d nonperiodic
6 = sample case for hex grid
"""


if 1 in options:  # time the sandpile solution
    init_64 = np.load("./tests/pile_64x64_init.npy")
    final_64 = np.load("./tests/pile_64x64_final.npy")
    init_128 = np.load("./tests/pile_128x128_init.npy")
    final_128 = np.load("./tests/pile_128x128_final.npy")

    t = tm.clock()
    solution_64 = au.sandpile(init_64)
    print('Time taken: %.5f' % (tm.clock()-t))
    print('Solutions match: ', (final_64 == solution_64).all())

    t = tm.clock()
    solution_128 = au.sandpile(init_128)
    print('Time taken: %.5f' % (tm.clock()-t))
    print('Solutions match: ', (final_128 == solution_128).all())


if 2 in options:  # time game of life
    state = np.random.randint(0, 10, [100, 100])
    t = tm.clock()
    final_state = au.life(state, 1000, False)
    print('Time Taken: ', tm.clock()-t)
    print('Solution: ', final_state)

if 3 in options:  # animate several gliders in 2d periodic
    glider = np.array([[0, 0, 0, 0, 0],
                       [0, 0, 1, 1, 0],
                       [0, 1, 0, 1, 0],
                       [0, 0, 0, 1, 0],
                       [0, 0, 0, 0, 0]])
    n = 1000
    state = np.zeros([101, 101])
    for i, j in zip(range(5, 100, 7), range(10, 100, 7)):
        state[i:j, i:j] = glider

    au.life_animate(state, n, True)
    print("Initial state: \n", state)
    print("Final state: \n", au.life(state, n, False))

if 4 in options:  # animate a generator in 2d periodic
    generator = np.array([[1, 1, 1, 0, 1],
                          [1, 0, 0, 0, 0],
                          [0, 0, 0, 1, 1],
                          [0, 1, 1, 0, 1],
                          [1, 0, 1, 0, 1]])
    n = 1000
    state = np.zeros([101, 101])
    state[48:53, 48:53] = generator

    au.life_animate(state, n, True)
    print("Initial state: \n", state)
    print("Final state: \n", au.life(state, n, False))

if 5 in options:  # animate a set of random initial condtions in 2d nonperiodic
    state = np.random.randint(0, 2, size=(100, 100))
    n = 1000

    au.life_animate(state, n, True)
    print("Initial state: \n", state)
    print("Final state: \n", au.life(state, n, False))

if 6 in options:  # sample case for hex grid
    test = [np.array([0, 0, 0, 0]),
            np.array([0, 1, 0, 1, 0]),
            np.array([0, 0, 0, 0]),
            np.array([0, 0, 0, 0, 0]),
            np.array([0, 0, 1, 0])]
    for _ in test:
        print(_)
    print('-'*25)
    for _ in au.lifehex(test, 1):
        print(_)

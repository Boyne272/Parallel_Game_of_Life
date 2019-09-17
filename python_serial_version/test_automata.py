#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Combination of tests for the automata module.

Created on Wed Oct 24 14:52:10 2018
@author: Richard Boyne rmb115@ic.ac.uk
"""

import numpy as np
import unittest as ut
import automata as au


class TestSandpile(ut.TestCase):
    "Has vairous test methods for the 2d sandpile model"

    def test_basic_sandpile(self):
        """Runs a simple case of an empty grid with a large pile in the center.

        Checks that the situation is fully resolved."""
        init = np.zeros([9, 9])
        init[4, 4] = 700
        solution = au.sandpile(init)
        unresolved = (solution >= 4).any()  # any piles still over 4
        self.assertFalse(unresolved)

    def test_random_sandpile(self):
        """Runs a random 9x9 grid with values from 1 to 10.

        Checks that the situation is fully resolved."""
        init = np.random.randint(0, 10, [9, 9])
        solution = au.sandpile(init)
        unresolved = (solution >= 4).any()  # any piles still over 4
        self.assertFalse(unresolved)

    def test_128(self):
        """Runs a given 64_64 grid example from the test/ directory.

        Checks the output with the given answer"""
        init = np.load("./tests/pile_128x128_init.npy")
        final = np.load("./tests/pile_128x128_final.npy")
        solution = au.sandpile(init)
        truth_array = np.equal(final, solution)
        self.assertTrue(truth_array.all())

    def test_64(self):
        """Runs a given 128_128 grid example from the test/ directory.

        Checks the output with the given answer"""
        init = np.load("./tests/pile_64x64_init.npy")
        final = np.load("./tests/pile_64x64_final.npy")
        solution = au.sandpile(init)
        truth_array = np.equal(final, solution)
        self.assertTrue(truth_array.all())


class TestLife2d(ut.TestCase):
    """Has vairous test methods for 2d game on life on period and fixed
    boundary conditions with both square and non-square grids.

    Shapes used were found at
    https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life"""

    def test_stationary(self, periodic=False):
        """Runs three stationary configureations for 100 iterations on
        fixed boundaries.

        Checks the final states match the intial"""
        tub = np.array([[0, 1, 0],
                        [1, 0, 1],
                        [0, 1, 0]])

        block = np.array([[0, 1, 1],
                          [0, 1, 1],
                          [0, 0, 0]])

        boat = np.array([[1, 1, 0],
                         [1, 0, 1],
                         [0, 1, 0]])

        for shape in [tub, block, boat]:
            state = np.zeros([5, 5])
            state[1:4, 1:4] = shape  # place shape in center
            final = au.life(state, 100, periodic)
            test = np.equal(state, final).all()  # true if the same
            self.assertTrue(test)

    def test_stationary_periodic(self):
        """Runs three stationary configureations for 100 iterations on
        periodic boundaries.

        This recycles the test_stationary code by passing
        in a different periodic setting.

        Checks the final states match the intial"""
        self.test_stationary(periodic=True)

    def test_oscillatory(self, periodic=False):
        """Runs a three oscillator states of period 2 for an even an odd
        number of iterations on fixed boundaries.

        Checks even matches the initial state and the odd does not"""
        blinker = np.array([[0, 0, 0, 0],
                            [1, 1, 1, 0],
                            [0, 0, 0, 0],
                            [0, 0, 0, 0]])

        toad = np.array([[0, 0, 0, 0],
                         [0, 1, 1, 1],
                         [1, 1, 1, 0],
                         [0, 0, 0, 0]])

        beacon = np.array([[1, 1, 0, 0],
                           [1, 0, 0, 0],
                           [0, 0, 0, 1],
                           [0, 0, 1, 1]])

        for shape in [blinker, toad, beacon]:
            state = np.zeros([6, 6])
            state[1:5, 1:5] = shape  # place shape in center
            final_eve = au.life(state, 100, periodic)  # even => unchanged
            final_odd = au.life(state, 101, periodic)  # odd => changed
            test_eve = np.equal(state, final_eve).all()  # true if the same
            test_odd = np.equal(state, final_odd).all()
            self.assertTrue(test_eve and not test_odd)

    def test_oscillatory_periodic(self):
        """Runs a three oscillator states of period 2 for an even an odd
        number of iterations on perodic boundaries.

        This recycles the test_oscillatory code by passing
        in a different periodic setting.

        Checks even matches the initial state and the odd does not"""
        self.test_oscillatory(periodic=True)

    def test_glider(self):
        """Runs a simple glider for 100 iterations on both fixed and
        periodic boundary condtitions with a non-square grid.

        Checks the glider still exists (i.e. 5 alive cells) for periodic and
        that the an expected stationary state for the glider hitting a wall
        (i.e. a box of 4 alive cells) is made"""
        glider = np.array([[0, 0, 0, 0, 0],
                           [0, 0, 1, 1, 0],
                           [0, 1, 0, 1, 0],
                           [0, 0, 0, 1, 0],
                           [0, 0, 0, 0, 0]])

        state = np.zeros([9, 15])  # gives an irregular shape
        state[2:7, 2:7] = glider  # place shape in center
        final_per = au.life(state, 100, periodic=True)  # should cross edge
        final_not = au.life(state, 100, periodic=False)  # should die at edge
        test_per = final_per.sum()  # should be 5 alive cells
        test_not = final_not.sum()  # should be 4 alive cells (block in corner)
        self.assertTrue(test_per == 5 and test_not == 4)

    def test_periodic(self):
        """Runs a 3x3 grid with 3 alive cells in any position on a
        periodic grid.

        Checks the outcome after 1 and 2 steps is that expected from
        periodic boundaries"""

        init = [1] * 3 + [0] * 6
        np.random.shuffle(init)
        init = np.array(init).reshape([3, 3])  # initialise randomly

        step1 = au.life(init, 1, periodic=True)  # should be all alive
        step2 = au.life(init, 2, periodic=True)  # should be all dead
        self.assertTrue(step1.sum() == 9 and step2.sum() == 0)


class TestLife3d(ut.TestCase):
    """Has vairous test methods for32d game on life on period and fixed
    boundary conditions with both square and non-square grids."""

    def test_stationary(self, periodic=False):
        """Runs four stationary configureations for 100 iterations on
        fixed boundaries.

        Checks the final states match the intial"""

        sh1 = np.array([[[0, 0, 0, 0],
                         [0, 1, 0, 0],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0]],
                        [[0, 1, 0, 0],
                         [1, 0, 1, 0],
                         [0, 1, 0, 0],
                         [0, 0, 0, 0]],
                        [[0, 0, 0, 0],
                         [0, 1, 0, 0],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0]],
                        [[0, 0, 0, 0],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0]]])

        sh2 = np.array([[[0, 1, 0, 0],
                         [0, 0, 0, 0],
                         [0, 1, 0, 0],
                         [0, 0, 0, 0]],
                        [[0, 1, 0, 0],
                         [1, 0, 1, 0],
                         [0, 1, 0, 0],
                         [0, 0, 0, 0]],
                        [[0, 0, 0, 0],
                         [0, 1, 0, 0],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0]],
                        [[0, 0, 0, 0],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0]]])

        sh3 = np.array([[[0, 0, 0, 0],
                         [0, 0, 1, 1],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0]],
                        [[0, 0, 1, 0],
                         [0, 0, 0, 1],
                         [0, 0, 1, 0],
                         [0, 0, 0, 0]],
                        [[0, 1, 0, 0],
                         [1, 0, 0, 0],
                         [0, 1, 0, 0],
                         [0, 0, 0, 0]],
                        [[0, 0, 0, 0],
                         [1, 1, 0, 0],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0]]])

        sh4 = np.array([[[0, 0, 0, 0],
                         [0, 0, 1, 1],
                         [0, 0, 1, 1],
                         [0, 0, 0, 0]],
                        [[0, 0, 0, 0],
                         [0, 0, 1, 1],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0]],
                        [[0, 0, 0, 0],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0]],
                        [[0, 0, 0, 0],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0],
                         [0, 0, 0, 0]]])

        for shape in [sh1, sh2, sh3, sh4]:
            state = np.zeros([6, 6, 6])
            state[1:5, 1:5, 1:5] = shape
            final = au.life3d(state, 100, periodic)
            test = np.equal(state, final).all()  # true if the same
            self.assertTrue(test)

    def test_stationary_periodic(self):
        """Runs four stationary configureations for 100 iterations on
        periodic boundaries.

        This recycles the test_stationary code by passing
        in a different periodic setting.

        Checks the final states match the intial"""
        self.test_stationary(periodic=True)

    def test_glider(self):
        """Runs a 3d simple glider for 100 iterations on both fixed and
        periodic boundary condtitions with a non-square grid.

        Checks the glider still exists for periodic and dies for the fixed
        boundary conditions"""
        glider = np.array([[[0, 0, 0, 0],
                            [0, 1, 1, 0],
                            [0, 1, 1, 0],
                            [0, 0, 0, 0]],
                           [[0, 1, 1, 0],
                            [1, 0, 0, 1],
                            [1, 0, 0, 1],
                            [0, 0, 0, 0]],
                           [[0, 0, 0, 0],
                            [0, 0, 0, 0],
                            [0, 0, 0, 0],
                            [0, 0, 0, 0]],
                           [[0, 0, 0, 0],
                            [0, 0, 0, 0],
                            [0, 0, 0, 0],
                            [0, 0, 0, 0]]])

        state = np.zeros([20, 8, 6])  # test an irregular grid
        state[1:5, 1:5, 1:5] = glider
        final_per = au.life3d(state, 100, periodic=True)  # should never die
        final_not = au.life3d(state, 100, periodic=False)  # should die at edge
        test_per = final_per.any()  # should be true
        test_not = final_not.any()  # should be false
        self.assertTrue(test_per and not test_not)

    def test_periodic(self):
        """Runs a 3x3x3 grid with 5 alive cells in any position on a
        periodic grid.

        Checks the outcome after 1 and 2 steps is that expected from
        periodic boundaries"""

        init = [1] * 5 + [0] * 22
        np.random.shuffle(init)
        init = np.array(init).reshape([3, 3, 3])  # initialise randomly

        step1 = au.life3d(init, 1, periodic=True)  # should be all alive
        step2 = au.life3d(init, 2, periodic=True)  # should be all dead
        self.assertTrue(step1.sum() == 27 and step2.sum() == 0)


class TestLifehex(ut.TestCase):
    """Has vairous test methods for 2d game on life on hexagonal grids.

    Test shapes found from: A discussion on possible rules for Life
    on hexagons: Bays. A Note on the Game of Life in Hexagonal and Pentagonal
    Tessellations. Complex Systems (2005)
    http://wpmedia.wolfram.com/uploads/sites/13/2018/02/15-3-4.pdf"""

    def test_glider5(self):
        """Runs the 5 period glider detailed in the above citation.

        Checks the output after 10 iterations is shifted twice to the right"""
        glider_start = [np.array([0, 0, 1, 0, 0, 0, 0, 0]),
                        np.array([0, 0, 0, 0, 1, 0, 0, 0, 0]),
                        np.array([1, 0, 1, 0, 0, 1, 0, 0]),
                        np.array([0, 0, 0, 0, 0, 0, 0, 0, 0]),
                        np.array([1, 0, 1, 0, 0, 1, 0, 0]),
                        np.array([0, 0, 0, 0, 1, 0, 0, 0, 0]),
                        np.array([0, 0, 1, 0, 0, 0, 0, 0])]

        glider_final = [np.array([0, 0, 0, 0, 1, 0, 0, 0]),
                        np.array([0, 0, 0, 0, 0, 0, 1, 0, 0]),
                        np.array([0, 0, 1, 0, 1, 0, 0, 1]),
                        np.array([0, 0, 0, 0, 0, 0, 0, 0, 0]),
                        np.array([0, 0, 1, 0, 1, 0, 0, 1]),
                        np.array([0, 0, 0, 0, 0, 0, 1, 0, 0]),
                        np.array([0, 0, 0, 0, 1, 0, 0, 0])]

        final = au.lifehex(glider_start, 10)
        test = []
        for i, row in enumerate(final):
            test.append(np.equal(row, glider_final[i]).all())
        self.assertTrue(all(test))

    def test_oscillator(self):
        """Runs a simple oscillator of period 2 for an even an odd
        number of iterations.

        Checks even matches the initial state and the odd does not"""
        oscillator = [np.array([0, 1, 0]),
                      np.array([0, 1, 0, 0]),
                      np.array([0, 0, 0])]

        final_eve = au.lifehex(oscillator, 100)
        final_odd = au.lifehex(oscillator, 101)
        test_eve, test_odd = [], []
        for i, row in enumerate(oscillator):
            test_eve.append(np.equal(row, final_eve[i]).all())
            test_odd.append(np.equal(row, final_odd[i]).all())
        self.assertTrue(all(test_eve) and not all(test_odd))


if __name__ == "__main__":  # run the tests if code exicuted independently
    ut.main()

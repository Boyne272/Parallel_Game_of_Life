#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
A collection of fucnctions designed to implement Cellular automata models for
Game of Life (on 2d, 3d and hexagonal grids) and Abelian Sandpile (in 2d)

Created on Thu Oct 18 15:02:31 2018
@author: Richard Boyne rmb115@ic.ac.uk
Submitted as part of MSc for Imperial College London Fri Oct 26 2018
"""
# %% Imports


import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as an


# %% Albien Sandpile


def sandpile(initial_state):
    """Generate final state for Abelian Sandpile in 2d with a critical value
    of 4 and sink boundary conditions.

    Parameters
    ---------
    initial_state : array_like or list of lists
        Initial 2d state of grid in an array of integers.

    Returns
    ------
    array_like
        Final state of grid in array of integers. """

    mask = np.array([1])  # to trigger first run
    init = np.array(initial_state)  # copy and ensure array form

    while mask.any():  # while we still have to collapse
        mask = init >= 4
        init -= 4 * mask  # subtract 4 from topple site
        init[:, 1:] += mask[:, :-1]  # shift right and add 1
        init[:, :-1] += mask[:, 1:]  # shift left
        init[1:, :] += mask[:-1, :]  # shift down
        init[:-1, :] += mask[1:, :]  # shift up
    return init


# %% Game of Life - 2d

def adj_period(state):
    """Find the number of adjasent alive cells along the axis=0,1 with
    periodic boundary conditions.

    Paramters
    ---------
    state : ndarray
        boolian array of alive cells

    Returns
    -------
    ndarray
        array of same shape as input"""

    adj = np.zeros(state.shape)

    right = np.roll(state, 1, axis=1)  # shift right
    left = np.roll(state, -1, axis=1)  # shift left

    adj += right
    adj += left
    adj += np.roll(state, 1, axis=0)   # shift down
    adj += np.roll(state, -1, axis=0)  # shift up
    adj += np.roll(right, -1, axis=0)  # shift right-up
    adj += np.roll(right, 1, axis=0)   # shift right-down
    adj += np.roll(left, -1, axis=0)  # shift left-up
    adj += np.roll(left, 1, axis=0)   # shift left-down

    return adj


def adj_nonperiod(state):
    """Find the number of adjasent alive cells along the axis=0,1 with
    fixed boundary conditions.

    Paramters
    ---------
    state : ndarray
        boolian array of alive cells

    Returns
    -------
    ndarray
        array of same shape as input"""

    adj = np.zeros(state.shape)

    adj[:, 1:] += state[:, :-1]  # shift right
    adj[:, :-1] += state[:, 1:]  # shift left
    adj[1:, :] += state[:-1, :]  # shift down
    adj[:-1, :] += state[1:, :]  # shift up

    adj[:-1, 1:] += state[1:, :-1]  # shift right-up
    adj[1:, 1:] += state[:-1, :-1]  # shift right-down
    adj[:-1, :-1] += state[1:, 1:]  # shift left-up
    adj[1:, :-1] += state[:-1, 1:]  # shift left-down

    return adj


def iterate(state, periodic):
    """Advance the 2d system by one time step for the 2,3/3 rule.

    Paramters
    ---------
    state : ndarray
        boolian array of alive cells

    Returns
    -------
    ndarray
        array of same shape as input"""

    if periodic is True:
        adj = adj_period(state)  # find the number of alive adjasent cells
    else:
        adj = adj_nonperiod(state)

    alive = (adj == 3).astype(int)  # all positions that must be alive
    stay = np.logical_and(adj == 2, state)  # all positions that stay alive
    return np.logical_or(stay, alive)  # combination of the two


def life(initial_state, nt, periodic=False):
    """Perform iterations of Conway’s Game of Life.

    Parameters
    ----------
    initial_state : array_like or list of lists
        Initial 2d state of grid in an array of booleans.
    nt : int
        Number of steps of Life to perform.
    periodic : bool
        If true, then grid is assumed periodic.

    Returns
    -------
    array_like
        Final state of grid in array of booleans """

    init = np.array(initial_state)  # copy and ensure array form

    for _ in range(nt):
        init = iterate(init, periodic)
    return init.astype(int)


class life_animate():
    """Create an instance to animate GOL in 2d.

    Class form is a convient way of keeping current satate of system accessable
    form all functions without global definitions.

    Pass parameters as though you were to call automata.life()"""

    def __init__(self, init_state, nt, periodic=False):
        "Create the attributes, setup figure and call FuncAnimation"
        self.current = init_state
        self.periodic = periodic

        self.fig = plt.figure(figsize=(10, 10))
        self.im = plt.imshow(init_state, animated=True, cmap='viridis')
        self.ax = plt.gca()
        n = init_state.shape[0]
        self.text = self.ax.text(0.1*n, 0.1*n, 'Time: 0', color='w')

        self.ani = an.FuncAnimation(self.fig, self.update, frames=range(nt),
                                    interval=10, blit=True)

    def update(self, f):
        "Iterate and update the figure with new state"
        self.current = iterate(self.current, self.periodic)
        self.im.set_array(self.current)
        self.text.set_text('Time: %i' % f)
        return self.im, self.text


# %% Game of life - 3d (some of 2d used here)


def adj_period3d(state):
    """Find the number of adjasent alive cells along the axis=0,1,2 with
    periodic boundary conditions.

    Calls automata.adj_period() after shifting along z axis.

    Paramters
    ---------
    state : ndarray
        boolian array of alive cells

    Returns
    -------
    ndarray
        array of same shape as input"""

    up_shift = np.roll(state, 1, axis=2)  # shift along the 3rd axis
    down_shift = np.roll(state, -1, axis=2)

    adj_0 = adj_period(state)
    adj_1 = adj_period(up_shift)
    adj_2 = adj_period(down_shift)
    adjs = [adj_0, adj_1, adj_2, up_shift, down_shift]
    # must add up,down shift as adj_period ignores the center cell
    return sum(adjs)


def adj_nonperiod3d(state):
    """Find the number of adjasent alive cells along the axis=0,1,2 with
    fixed boundary conditions.

    Calls automata.adj_nonperiod() after shifting along z axis.

    Paramters
    ---------
    state : ndarray
        boolian array of alive cells

    Returns
    -------
    ndarray
        array of same shape as input"""

    up_shift = state[:, :, 1:]  # shift along the 3rd axis
    down_shift = state[:, :, :-1]

    adj = adj_nonperiod(state)
    adj_up = adj_nonperiod(up_shift)
    adj_down = adj_nonperiod(down_shift)

    adj[:, :, :-1] += up_shift + adj_up
    adj[:, :, 1:] += down_shift + adj_down
    # must add up,down shift as adj_period ignores the center cell

    return adj


def iterate3d(state, periodic):
    """Advance the 3d system by one time step for the 5,4/5 rule.

    Paramters
    ---------
    state : ndarray
        boolian array of alive cells

    Returns
    -------
    ndarray
        array of same shape as input"""

    if periodic is True:
        adj = adj_period3d(state)  # find the number of alive adjasent cells
    else:
        adj = adj_nonperiod3d(state)

    alive = (adj == 5).astype(int)  # all positions that must be alive
    stay = np.logical_and(adj == 4, state)  # all positions that stay alive
    return np.logical_or(stay, alive)  # combination of the two


def life3d(initial_state, nt, periodic=False):
    """Perform iterations of Conway’s Game of Life in 3d.

    Parameters
    ----------
    initial_state : array_like or list of lists
        Initial 2d state of grid in an array of booleans.
    nt : int
        Number of steps of Life to perform.
    periodic : bool
        If true, then grid is assumed periodic.

    Returns
    -------
    array_like
        Final state of grid in array of booleans """

    init = np.array(initial_state)  # copy and ensure array form

    for _ in range(nt):
        init = iterate3d(init, periodic)
    return init.astype(int)


# %% Game of Life - Hexagonal Grid


def adj_hex(state):
    """Find the number of adjasent alive cells on a hexagonal grid.

    Paramters
    ---------
    initial_state : list of lists of booleans
        Initial 2d state of grid on hexes.

    Returns
    -------
    list of arrays"""

    adj = [np.zeros(row.size) for row in state]
    for i, row in enumerate(state):
        adj[i][1:] += row[:-1]  # shift row left
        adj[i][:-1] += row[1:]  # shift row right

        if i % 2 == 0:  # even => shorter row
            if i != 0:  # not the first row
                adj[i] += state[i-1][1:]  # add the upper row
                adj[i] += state[i-1][:-1]
            if i != len(state)-1:  # not the last row
                adj[i] += state[i+1][1:]  # add the lower row
                adj[i] += state[i+1][:-1]

        else:  # odd => loger row
            adj[i][:-1] += state[i-1]  # add the upper row
            adj[i][1:] += state[i-1]
            if i != len(state)-1:  # not the last row
                adj[i][:-1] += state[i+1]  # add the lower row
                adj[i][1:] += state[i+1]
    return adj


def iterate_hex(state):
    """Advance the hexagonal system by one time step for the 3,5/2 rule.

    Paramters
    ---------
    initial_state : list of lists of booleans
        Initial 2d state of grid on hexes.

    Returns
    -------
    list of arrays"""

    adj = adj_hex(state)  # find the number of alive adjasent cells
    alive = []

    for adj_row, alive_row in zip(adj, state):
        dead = np.logical_not(alive_row)
        revive = np.logical_and(adj_row == 2, dead)  # revived cells
        tmp = np.logical_or(adj_row == 3, adj_row == 5)
        stay = np.logical_and(tmp, alive_row)  # staying alive cells
        alive.append(np.logical_or(revive, stay))  # all living cells

    return alive


def lifehex(init_state, nt):
    """ Perform iterations of Conway’s Game of Life on a hexagonal grid.

    Parameters
    ----------
    initial_state : list of lists of booleans
        Initial 2d state of grid on hexes.
    nt : int
        Number of steps of Life to perform.

    Returns
    -------
    list of arrays
        Final state of grid."""

    state = init_state.copy()  # ensure don't affect input
    for _ in range(nt):
        state = iterate_hex(state)
    return state

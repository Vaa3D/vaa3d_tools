.. NeuroMaC documentation master file, created by
   sphinx-quickstart2 on Thu Aug 21 17:31:33 2014.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

NeuroMaC's documentation
=========================

*NeuroMaC is a phenomenological, computational framework to generate \
large numbers of virtual neuronal morphologies (and resultant \
microcircuits) simultaneously according to growth-rules expressed in \
terms of interactions with the environment.*

- **NeuroMaC**: Neuronal Morphologies & Circuits
- **Computational framework**: a suite of software tools built around \
  a central concept
- **Virtual neuronal morphologies**: 3D digital descriptions of \
  neuronal shape (both axons and dendrites)
- **Microcircuits**: Morphologies are generated together in a \
  simulated volume. With the addition of connections rules circuits emerge.
- **Interactions**: Growth-cones branch, terminate and elongate. Each \
  of these steps can be influenced by environmental cues. Most obvious \
  is guidance through repulsion and attraction to simulated cues.
- **Phenomenological**: Neither biochemical pathways nor physics are \
  simulated. As such, growth is purely phenomenological. NeuroMaC is \
  not a simulator of actual neuronal development.

.. warning:: Currenly, a prototype of NeuroMaC is implemented in Python. \
  This version is a proof-of-principle and nothing beyond that. This \
  prototype has many limitations and we are working towards a \
  non-prototype version, which should be released in the next year. \
  The current prototype code is freely available.

Contents
========

.. toctree::
   :maxdepth: 3

   neuromac
   docu
   examples
   api
   problems
   
Reference
=========

When using NeuroMaC, please refer to the following paper:

Benjamin Torben-Nielsen & Erik De Schutter. *Context-aware modelling \
of neuronal morphologies*. Frontiers in Neuroanatomy,  8:92. \
doi: 10.3389/fnana.2014.00092.

License
=======

NeuroMac - Neuronal Morphologies and Circuits

Copyright (C) 2014 Ben Torben-Nielsen & Okinawa Institute of Science and Technology, Japan.,

NeuroMac is free software: you can redistribute it and/or modify it \
under the terms of the GNU General Public License as published by the \
Free Software Foundation, either version 3 of the License, or \
(at your option) any later version.

NeuroMaC is distributed in the hope that it will be useful, but \
WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the \
GNU General Public License for more details.

You should have received a copy of the GNU General Public License \
along with this program. If not, see \
`http://www.gnu.org/licenses/ <http://www.gnu.org/licenses>`_.


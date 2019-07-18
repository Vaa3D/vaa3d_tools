Problem solving
===============

* Sometimes TCP ports are not closed correctly.
  A: :code:`netstate -tulpn` tells you which process/PID uses a certain port. 
  :code:`kill -9` it.

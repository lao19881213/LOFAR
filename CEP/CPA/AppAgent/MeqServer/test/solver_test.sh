#!/bin/sh
if [ ! -L meqserver ]; then
  ln -s ../src/meqserver .
fi
if [ ! -L meqserver.g ]; then
  ln -s ${srcdir}/../src/meqserver.g .
fi
if [ ! -L parmtable.g ]; then
  ln -s ${srcdir}/../../../PSS4/MEQ/src/parmtable.g .
fi
glish -l ${srcdir}/solver_test.g -runtest &>solver_test.log
retval=$?
cat solver_test.log
echo "Glish exited with status $retval"
exit $retval

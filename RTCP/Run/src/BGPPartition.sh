#!/bin/bash

. locations.sh

function start() {
  mpirun -partition $PARTITION -timeout 300 -nofree -exe /bgsys/tools/hello >/dev/null
}

function stop() {
  mpirun -partition $PARTITION -free wait
}

function getpid() {
  STATUS=`bgpartstatus $PARTITION </dev/null`

  case $STATUS in
    busy) PID=UP
          ;;
    *)    PID=DOWN
          ;;
  esac        
}

function setpid() {
  true
}

function delpid() {
  true
}

. controller.sh

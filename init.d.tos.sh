#!/bin/sh
start() {
    tos&
}
stop() {
    killall -9 tos
}
case "$1" in
  start)
    start
    ;;
  stop)
    stop
    ;;
  retart)
    stop
    start
    ;;
  *)
    echo "Usage: $0 {start|stop|restart|uninstall}"
esac

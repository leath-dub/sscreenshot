#!/bin/sh

prev=$SS_DIR
export SS_DIR=$PWD

is_png()
{
  tst=$(hexdump $1 | head -1 | awk '{print $2, $3}')
  [ "$tst" = "5089 474e" ] && return 0
  return 1
}

test()
{
  type sss && {

    # test 1
    sss && {
      is_png ./screenshot.png || {
        echo "failed at test 1"
        exit 1
      }
    }

    # test 2
    sss -n && {
      is_png ./screenshot.png || {
        echo "failed at test 2"
        exit 1
      }
    }

    # test 3
    sss -f && {
      is_png ./screenshot.png || {
        echo "failed at test 3"
        exit 1
      }
    }

    # test 4
    sss -n test.png && {
      is_png ./test.png || {
        echo "failed at test 4"
        exit 1
      }
    }

    # test 5
    sss -f test.png && {
      is_png ./test.png || {
        echo "failed at test 5"
        exit 1
      }
    }

    cleanup
  }
}

cleanup()
{
  rm -f ./test.png ./screenshot.png ./fullscreen.png
}

test
export SS_DIR=$prev

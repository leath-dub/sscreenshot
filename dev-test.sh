#!/usr/bin/zsh

prev=$SS_DIR
export SS_DIR=$(pwd)

test()
{
  which sss && \
    sss && [[ $(hexdump ./screenshot.png | head -1 | awk '{print $2,$3}') == "5089 474e" ]] \
    && sss -n && [[ $(hexdump ./screenshot.png | head -1 | awk '{print $2,$3}') == "5089 474e" ]] \
    && sss -f && [[ $(hexdump ./fullscreen.png | head -1 | awk '{print $2,$3}') == "5089 474e" ]] \
    && sss -n test.png && [[ $(hexdump ./test.png | head -1 | awk '{print $2,$3}') == "5089 474e" ]] \
    && sss -f test.png && [[ $(hexdump ./test.png | head -1 | awk '{print $2,$3}') == "5089 474e" ]] \
    && cleanup
}

cleanup()
{
  rm -f ./test.png ./screenshot.png ./fullscreen.png
}

test
export SS_DIR=$prev

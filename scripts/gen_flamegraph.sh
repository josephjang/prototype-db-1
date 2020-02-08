perf script > out.perf
stackcollapse-perf.pl out.perf > out.folded
flamegraph.pl out.folded  > out.svg

#firefox out.svg
google-chrome out.svg

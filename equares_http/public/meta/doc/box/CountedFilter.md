The box passes each $n$-th data frame from its **input** port to the **output** port; $n$ is the last value arrived at port **count**. If there is no data available at **count** yet,
the processing of **input** is cancelled.

Notice that when a new data frame comes to the **count** port, the internal filter counter is reset.
Thus, after setting filter count, first $n-1$ data frames on **input** will be skipped, then $n$-th data frame will be passed, then $n-1$ frames will be skipped again, and so on.

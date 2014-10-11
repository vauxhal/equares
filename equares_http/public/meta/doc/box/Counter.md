The box holds an internal counter that increases by one each time a new data frame comes to the **input** port.
The initial value of the counter is zero. Each time the counter increases, its value is sent to the **count** port.

A data frame coming to the **reset** port causes the internal counter to take zero value. That value is not sent to the **count** port.

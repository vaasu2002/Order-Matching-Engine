# Time In Force (TIF)

| Name                      | Decimal | Binary Bit | Meaning                                                               |
| ------------------------- | ------: | ---------: | --------------------------------------------------------------------- |
| NO_CONDITIONS (DAY)       |       0 |    `00000` | Active till end of day                                                |
| ALL_OR_NONE (AON)         |       1 |    `00001` | Must fill entire quantity or don’t fill at all (but can sit)          |
| IMMEDIATE_OR_CANCEL (IOC) |       2 |    `00010` | Whatever fills now fills, rest cancels. does not rest                 |
| FILL_OR_KILL (FOK)        |       3 |    `00011` | AON + IOC together. must fill entire amount immediately or cancel     |
| GOOD_TILL_CANCELED (GTC)  |       4 |    `00100` | remains active until it is executed or manually                       |
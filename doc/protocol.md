
# Client -> Server register packet

## Request
| Field     | start byte    | length (bytes) | description                           |
|-----------| ---------     | -------------  | ------------                          |
| passcrc32 | 0             | 4              | password hashed as crc32 used to login lobby          |
| username  | 4             | 8              | username of specific client           |


## Response
| Field     | start bit     | length (bits) | description                            |
|-----------| ---------     | ------------- | ------------                           |
| status    | 0             | 2             | 0 = OK, 1 = FAIL_PASSW, 2 = LOBBY_FULL |
| reserved  | 2             | 2             |                                        |
| color_id  | 4             | 4             | color id of snake body                 |
| body_ascii| 8             | 8             | printable character as snake body      |


# Server -> Client configs packet

## Request
| Field       | start bit     | length (bits) | description                            |
|-----------  | ---------     | ------------- | ------------                           |
| username    | 0             | 80            | player username                        |
| color_id    | 0             | 4             | color id of snake body player          |
| ready_flag  | 0             | 1             | ready state of player                  |
| reserved    | 0             | 3             |                                        |
| body_ascii  | 0             | 8             | body ascii of player                   |

## Response to all sockets
| Field       | start bit     | length (bits) | description                            |
|-----------  | ---------     | ------------- | ------------                           |
| count       | 0             | 8             | how many config there is in this packet|
| username_1  | 0             | 80            | player1 username                       |
| color_id_1  | 0             | 4             | color id of snake body player1         |
| ready_flag_1| 0             | 1             | ready state of player1                 |
| reserved    | 0             | 3             |                                        |
| body_ascii_1| 0             | 8             | body ascii of player1                  |
| ...         | ...           | ...           |   ...                                  |
| username_n  | n             | x             |                                        |
| color_id_n  | n             | x             |                                        |
| body_ascii_n| n             | x             |                                        |
| ready_flag_n| n             | x             |                                        |




<!-- # Client -> Server ingame packet
| Field     | start bit     | length (bits) |
|-----------| ---------     | ------------- |
| direction | 0             | 2             |
| direction | 0             | 2             |

# Server -> Client status
| Field     | start bit     | length (bits) |
|-----------| ---------     | ------------- |
| direction | 0             | 2             |
| direction | 0             | 2             | -->

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
| arena_w   | 16            | 8             | arena width                            |
| arena_h   | 24            | 8             | arena height                           |

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


# Server -> Client game packet

## DATAGRAM
| Field       | start bit     | length (bits) | description                            |
|-----------  | ---------     | ------------- | ------------                           |
| packet_type | 0             | 2             | type identifying datagram  (0x00)      |
| snake_count | 2             | 6             | snake count in body                    |
|-----------  | ---------     | ------------- | ------------                           |
| snake_id    | 0             | 16            | ID identifying specific snake player   |
| snakeb_len  | 16            | 9             | buffer length of one snake body        |
| starting_x  | 25            | 1             | Tells what is indicating first byte    |
| reserved    | 26            | 6             | RESERVED                               |
|-----------  | ---------     | ------------- | ------------                           |
| body_buffer | 24            | snakeb_len    | compressed snake body                  |
| ...         | ...           | ...           | ...                                    |

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
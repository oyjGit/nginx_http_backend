.. Drizzle Client & Protocol Library

.. Copyright (C) 2008 Eric Day (eday@oddments.org)
.. All rights reserved.
 
.. Use and distribution licensed under the BSD license.  See
.. the COPYING.BSD file in the root source directory for full text.

Drizzle Protocol
================

`This is currently a proposed draft as of November 29, 2008`

The Drizzle protocol works over TCP, UDP, and Unix Domain Sockets
(UDS, also known as IPC sockets), although there are limitations when
using UDP (this is discussed below). In the case of TCP and UDS,
a connection is made, a command is sent, and a response loop is
started. Socket communication ends when either side closes the
connection or a QUIT command is issued.

TCP and UDS communications will be full duplex. This means that as
the client is sending a command, it is possible for the server to
report an error before the sending of data completes. This allows
the server to do preliminary checks (table exists, authentication,
...) before a request is completely sent so the client may abort. This
will primarily be used for large requests (INSERTing large BLOBs).

TCP and UDS communications will also allow for pipe-lining of requests
and concurrent command execution. This means a client does not need
to wait for a command to finish before a new command is sent. It is
even possible a later command issued will complete and have a result
before an earlier command. Result packets may be interleaved so a
client issuing concurrent commands must be able to parse results
concurrently.

UDP sockets are supported to allow small, fast updates for
applications such as statistical gathering. Since UDP does not
guarantee delivery, this method should not be used for applications
that require reliable transport. When using UDP, the authentication
packet (if needed) and command packet are bundled into a single UDP
packet and sent. This puts a limitation on the size of the request
being made, and this limit can be different between network hosts. The
absolute limit is 65,507 bytes (28 bytes used for IPv4 and UDP
headers), but again, this can depend on the network hosts. Responses
are optional when issuing UDP commands, and this preference is
specified in the handshake packet.

All sizes given throughout this document are in bytes. Byte order
for all multi-byte binary objects such as lengths and mutli-byte
bit-fields are packed little-endian.


Packet Sequence Overview
------------------------

The sequence of packets for a simple connection and command that
responds with an OK packet:

C: Command
S: OK

The sequence of packets for a simple connection and query command
with results:

C: Command
S: OK
S: Fields (optional, multiple packets)
S: Rows (multiple packets)
S: EOF

When authentication is required for a command, the server will ask
for it. For example:

C: Command
S: Authentication Required
C: Authentication Credentials
S: OK    
S: Fields
S: Rows
S: EOF

The server will use the most recent credential information when
processing subsequent commands.

If a client wishes to multiplex commands on a single connection,
it can do so using the command identifiers. Here is an example of
how the packets could be ordered, but this will largely depend on
the servers ability to process the commands concurrently and the
processing time for each command.

C: Command (Command ID=1)
C: Command (Command ID=2)
S: OK (Command ID=2)
S: Field (Command ID=2)
S: OK (Command ID=1)
S: Fields (Command ID=2)
S: Rows (Command ID=2)
S: EOF

As you can see, the commands may be executed with results generated
in any order, and the packet containing the results may be interleaved.


Length Encoding
---------------

Some lengths used within the protocol packets are length encoded. This
means the size of the length field will vary between 1 and 9 bytes,
and is determined by the value of the first byte.

0-252 - Actual length
253   - NULL value (only applicable in row results)
254   - Following 8 bytes contain actual length
255   - Depends on context, usually signifies end


Packets
-------

Packets consist of two layers. The first is meant to be small,
simple, and have just enough information for fast router and proxy
processing. It consists of a fixed-size part, along with a variable
sized client id (explained later), a series of chunked data, followed
by a checksum at the end. The chunked transfer encoding allows for
not having to pre-compute the packet data length before sending,
and support packets of any size. It also allows for a large packet
to be aborted gracefully (without having to close the connection)
in the event of an error.

   +-------------------------------------------------------------------------+
   +                                  32 Bits                                +
   +-------------------------------------------------------------------------+
    
   +-----+----------------+----------------+---------------------------------+
   |   0 | Magic          | Protocol       | Command ID                      |
   +-----+----------------+----------------+---------------------------------+

   +-----+---------------------------------+---------------------------------+
   |  32 | Command / Result Code           | Client ID Length                |
   +-----+---------------------------------+---------------------------------+

   +-----+---------------------------------+---------------------------------+
   |  64 | Client ID (optional, variable length)                             |
   +-----+---------------------------------+---------------------------------+

   +-----+---------------------------------+---------------------------------+
   | 64+ | Chunk Length and Value Pairs (optional, variable length)          |
   +-----+---------------------------------+---------------------------------+

   +-----+---------------------------------+---------------------------------+
   + 64+ | Chunk Length = 0                |                                 |
   +-----+---------------------------------+---------------------------------+

   +-----+---------------------------------+---------------------------------+
   | 80+ | Checksum                                                          |
   +-----+---------------------------------+---------------------------------+

The first part of a packet is:

1-byte Magic number, the value should be 0x44.

1-byte Protocol version, currently 1.

2-byte Command ID. This is a unique number among all other queries
       currently being executed on the connection. The client is
       responsible for choosing a unique number while generating a
       command packet, and all response packets associated with that
       command must have the same command ID. Once a command has been
       completed, the client may reuse the ID.

2-byte Command/result code. For commands, this may be:

       1  ECHO
         The entire packet is simply echoed back to the caller.
       2  SET
         Set protocol options.
       3  QUERY
         Execute query.
       4  QUERY_RO
         Same as QUERY, but hints that this is a read-only
         query. This is only useful for routers/proxies who may want
         to redirect the request to a read slave.

       Result codes may be:

       1  OK
         Single packet success response. No data associated
         with the result besides parameters.
       2  ERROR
         Single packet error response.
       3  DATA
         Start of a multi-packet result set.
       3  DATA_END
         Mark the end of a series of data packets. This is
         useful so a low level router or proxy can know when a
         response is complete without inspecting the contents of
         the packets.

2-byte Client ID length.
X-byte Client ID (length is value of client ID length).
The client ID is there for the client and routers/proxies to use. The server
treats this as opaque data, and will only preserve it to send
in responses. This can be used as a sharding key, to keep
state information in a proxy, or any other use.

Next, zero or more chunks are given, terminated by a chunk length of
0. Each chunk consist of a length and then that amount of data.

2-byte Chunk length
X-byte Chunk (length is value of chunk length)

After the the chunk length of 0 is given, a checksum value is given
that was computed for the entire packet.

4-byte Checksum

The second layer of the protocol is encapsulated inside of the
chunked encoding. This consists of zero or more packet parameters,
an end of parameter marker, followed by an optional data set that is
given until the end of a packet (or the end of all chunks).


Packet Parameters
-----------------

Packet parameter names are defined in a global namespace, although
not all parameters are relevant for all packet types. Parameters are
enumerated, and the name is specified with a 1-byte value representing
the enumerated name. Each packet parameter may have a value associated
with it, and each parameter defines the size and how that value is
given. The list of possible packet parameters are:

0   END_OF_PARAMETERS - Marks the end of a parameter list.

Parameters used for setting options:

1   AUTH              - 1-byte value with authentication mechanism
                        to use. Possible values are:
                        0 - None.
                        1 - MD5 on user and password.
                        2 - 3-way handshake.
2   CHECKSUM          - 1-byte value with preferred checksum
                        type. Possible values are:
                        0 - None.
                        1 - CRC32
3   COMPRESSION       - 1-byte value with preferred compression
                        type. Possible values are:
                        0 - None.
                        1 - zlib.
                        2 - bzip2.
4   FIELD_ENCODING    - 1-byte value with preferred field encoding
                        type. Possible values are:
                        0 - String.
                        1 - Native.
5   FIELD_INFO        - 1-byte value to determine if field information
                        should be sent. Possible values are:
                        0 - None.
                        1 - Send field info.

(6-63 Reserved for future options that can be set)

Parameters used in responses:

64  STATUS            - 4-byte bit field.
65  NUM_ROWS_AFFECTED - Length-encoded count of rows affected.
66  NUM_ROWS_SCANNED  - Length-encoded count of rows scanned.
67  NUM_WARNINGS      - Length-encoded count of warnings encountered.
68  INSERT_ID         - Last insert ID.
69  ERROR_CODE        - 4-byte error code.
70  ERROR_STRING      - Length-encoded string.
71  SQL_STATE         - Length-encoded string.
72  NUM_FIELDS        - 4-byte integer.
73  FIELD_START       - No value, starts a new set of field parameters.
74  FIELD_TYPE        - 2-byte enumerated type.
75  FIELD_LENGTH      - Length-encoded value.
76  FIELD_FLAGS       - 4-byte bit-field.
77  DB_NAME           - Length-encoded string.
78  TABLE_NAME        - Length-encoded string.
79  ORIG_TABLE_NAME   - Length-encoded string.
80  FIELD_NAME        - Length-encoded string.
81  ORIG_FIELD_NAME   - Length-encoded string.
82  DEFAULT_VALUE     - Length-encoded string.

(83-255 Reserved for future responses parameters)

"Length-encoded string" means a length-encoded value, followed by a
string of that length.


Command
-------

Inside of the chunked data, command packets consist of zero or more
parameters depending on which options are being set, followed by
a end of parameter marker, and then all data until the end of the
chunks are considered arguments for the command. For a QUERY, this
will be the actual query to run.


OK/ERROR
--------

The server responds with an OK or ERROR if no row data is given. A
list of parameters may follow, and the marked with an end of parameter
value.


DATA
----

A data packet consists of a series of parameters, followed by the end
of parameter, and then a series of length-encoded values holding field
values. The NUM_FIELDS parameter must be given before any values, as
this indicates when a start of a new row happens. The field values may
either be in string format or native data type, depending on the value
of FIELD_ENCODING.

There may be multiple rows inside of a single DATA result packet. In
the case of large result sets, the result should be split into multiple
DATA packets since other concurrent commands on the connection will
block if a single large packet is sent. By breaking resulting rows
into multiple DATA packets, other commands are then allowed to send
interleaved response packets.

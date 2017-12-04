# Redis-ZsetExtCmd
This redis module implements the zset pop function with condition. This project is intended for learning purpose. The same effect of calling the command can also be achieved by using lua script.

## How to build
Just `cd` into the src dir and run `make qzZsetExtCmd.so`.

## Command
### QZ.ZLPOPIF key [condition]
Remove the **first** element in zset and return it. If condition specified, only remove the first element if its score **less than or equal to** the condition. Note even if there are multiple elements meet the condition, only the first one will be removed at a time.

#### Return value
Array reply: the removed element and its score.  
Returns NIL if the key not exist, or the zset is empty, or the condition is not satisfied.

#### Examples
    127.0.0.1:6479> zadd myzset 1 a
	(integer) 1
	127.0.0.1:6479> zadd myzset 2 b
	(integer) 1
	127.0.0.1:6479> zadd myzset 3 c
	(integer) 1
	127.0.0.1:6479> qz.zlpopif myzset
	1) "a"
	2) "1"
	127.0.0.1:6479> qz.zlpopif myzset 1
	(nil)
	127.0.0.1:6479> qz.zlpopif myzset 2
	1) "b"
	2) "2"
	127.0.0.1:6479> zrange myzset 0 -1
	1) "c"

### QZ.ZRPOPIF key [condition]
Remove the **last** element in zset and return it. If condition specified, only remove the first element if its score **greater than or equal to** the condition. Note even if there are multiple elements meet the condition, only the last one will be removed at a time.

#### Return value
Array reply: the removed element and its score.  
Returns NIL if the key not exist, or the zset is empty, or the condition is not satisfied.

#### Examples
	127.0.0.1:6479> zadd myzset 1 a
	(integer) 1
	127.0.0.1:6479> zadd myzset 2 b
	(integer) 1
	127.0.0.1:6479> zadd myzset 3 c
	(integer) 1
	127.0.0.1:6479> qz.zrpopif myzset
	1) "c"
	2) "3"
	127.0.0.1:6479> qz.zrpopif myzset 5
	(nil)
	127.0.0.1:6479> qz.zrpopif myzset 1
	1) "b"
	2) "2"
	127.0.0.1:6479> zrange myzset 0 -1
	1) "a"

## License
This project is licensed under MIT, see LICENSE file.

.entry LABEL
LABEL: .string "abcdef"  hj
mov 3, 6
cmp @r5 @r6
lea 78, LABEL
L1: bne 78, @r4
.extern L2
sub L1 ,, @r5
dec
stop 1
L1: .data -4
L3: .data ,1,2,3,4 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789

L4: .entry L3
.extern L1
.string "hello"
L5: .data 45, -3000

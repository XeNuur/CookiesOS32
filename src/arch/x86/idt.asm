[bits 32]

;void general_interrupt_handler(struct interrupt_frame*);
extern general_exception_handler_err
extern general_exception_handler

global load_idt
load_idt:
   ;New call frame
   push ebp
   mov ebp, esp
   
   ;Load our bug chungus
   mov eax, [ebp + 8]
   lidt [eax]

   ;Restore old call frame
   mov esp, ebp
   pop ebp

   ret

;Isr exeptions
%macro isr_noerr 1
isr%1:
    call general_exception_handler
    iret
%endmacro
%macro isr_err 1
isr%1:
    call general_exception_handler_err
    iret
%endmacro

isr_noerr 0
isr_noerr 1
isr_noerr 2
isr_noerr 3
isr_noerr 4
isr_noerr 5
isr_noerr 6
isr_noerr 7
isr_err 8
isr_noerr 9
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14
isr_noerr 15
isr_noerr 16
isr_noerr 17
isr_noerr 18
isr_noerr 19
isr_noerr 20
isr_noerr 21
isr_noerr 22
isr_noerr 23
isr_noerr 24
isr_noerr 25
isr_noerr 26
isr_noerr 27
isr_noerr 28
isr_noerr 29
isr_noerr 30
isr_noerr 31

;;other iqr for the os
isr_noerr 32
isr_noerr 33
isr_noerr 34
isr_noerr 35
isr_noerr 36
isr_noerr 37
isr_noerr 38
isr_noerr 39
isr_noerr 40
isr_noerr 41
isr_noerr 42
isr_noerr 43
isr_noerr 44
isr_noerr 45
isr_noerr 46
isr_noerr 47
isr_noerr 48
isr_noerr 49
isr_noerr 50
isr_noerr 51
isr_noerr 52
isr_noerr 53
isr_noerr 54
isr_noerr 55
isr_noerr 56
isr_noerr 57
isr_noerr 58
isr_noerr 59
isr_noerr 60
isr_noerr 61
isr_noerr 62
isr_noerr 63
isr_noerr 64
isr_noerr 65
isr_noerr 66
isr_noerr 67
isr_noerr 68
isr_noerr 69
isr_noerr 70
isr_noerr 71
isr_noerr 72
isr_noerr 73
isr_noerr 74
isr_noerr 75
isr_noerr 76
isr_noerr 77
isr_noerr 78
isr_noerr 79
isr_noerr 80
isr_noerr 81
isr_noerr 82
isr_noerr 83
isr_noerr 84
isr_noerr 85
isr_noerr 86
isr_noerr 87
isr_noerr 88
isr_noerr 89
isr_noerr 90
isr_noerr 91
isr_noerr 92
isr_noerr 93
isr_noerr 94
isr_noerr 95
isr_noerr 96
isr_noerr 97
isr_noerr 98
isr_noerr 99
isr_noerr 100
isr_noerr 101
isr_noerr 102
isr_noerr 103
isr_noerr 104
isr_noerr 105
isr_noerr 106
isr_noerr 107
isr_noerr 108
isr_noerr 109
isr_noerr 110
isr_noerr 111
isr_noerr 112
isr_noerr 113
isr_noerr 114
isr_noerr 115
isr_noerr 116
isr_noerr 117
isr_noerr 118
isr_noerr 119
isr_noerr 120
isr_noerr 121
isr_noerr 122
isr_noerr 123
isr_noerr 124
isr_noerr 125
isr_noerr 126
isr_noerr 127
isr_noerr 128
isr_noerr 129
isr_noerr 130
isr_noerr 131
isr_noerr 132
isr_noerr 133
isr_noerr 134
isr_noerr 135
isr_noerr 136
isr_noerr 137
isr_noerr 138
isr_noerr 139
isr_noerr 140
isr_noerr 141
isr_noerr 142
isr_noerr 143
isr_noerr 144
isr_noerr 145
isr_noerr 146
isr_noerr 147
isr_noerr 148
isr_noerr 149
isr_noerr 150
isr_noerr 151
isr_noerr 152
isr_noerr 153
isr_noerr 154
isr_noerr 155
isr_noerr 156
isr_noerr 157
isr_noerr 158
isr_noerr 159
isr_noerr 160
isr_noerr 161
isr_noerr 162
isr_noerr 163
isr_noerr 164
isr_noerr 165
isr_noerr 166
isr_noerr 167
isr_noerr 168
isr_noerr 169
isr_noerr 170
isr_noerr 171
isr_noerr 172
isr_noerr 173
isr_noerr 174
isr_noerr 175
isr_noerr 176
isr_noerr 177
isr_noerr 178
isr_noerr 179
isr_noerr 180
isr_noerr 181
isr_noerr 182
isr_noerr 183
isr_noerr 184
isr_noerr 185
isr_noerr 186
isr_noerr 187
isr_noerr 188
isr_noerr 189
isr_noerr 190
isr_noerr 191
isr_noerr 192
isr_noerr 193
isr_noerr 194
isr_noerr 195
isr_noerr 196
isr_noerr 197
isr_noerr 198
isr_noerr 199
isr_noerr 200
isr_noerr 201
isr_noerr 202
isr_noerr 203
isr_noerr 204
isr_noerr 205
isr_noerr 206
isr_noerr 207
isr_noerr 208
isr_noerr 209
isr_noerr 210
isr_noerr 211
isr_noerr 212
isr_noerr 213
isr_noerr 214
isr_noerr 215
isr_noerr 216
isr_noerr 217
isr_noerr 218
isr_noerr 219
isr_noerr 220
isr_noerr 221
isr_noerr 222
isr_noerr 223
isr_noerr 224
isr_noerr 225
isr_noerr 226
isr_noerr 227
isr_noerr 228
isr_noerr 229
isr_noerr 230
isr_noerr 231
isr_noerr 232
isr_noerr 233
isr_noerr 234
isr_noerr 235
isr_noerr 236
isr_noerr 237
isr_noerr 238
isr_noerr 239
isr_noerr 240
isr_noerr 241
isr_noerr 242
isr_noerr 243
isr_noerr 244
isr_noerr 245
isr_noerr 246
isr_noerr 247
isr_noerr 248
isr_noerr 249
isr_noerr 250
isr_noerr 251
isr_noerr 252
isr_noerr 253
isr_noerr 254
%define isr_stub_count 255

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    isr_stub_count
    dd isr%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep


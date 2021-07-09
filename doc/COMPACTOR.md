# `compactor` Dokumentation


## Workflow:

* Create read-only array data description file (input file)
* Call `compactor` with input file
* Use/adapt output file 

## Data Alignment (optional)

Data alignment is supported, i.e. given alignments for input arrays are assured relative to the output array. But handle alignments with care: for alignments >1 the resulting compacted array can even be larger than the size of the input arrays. In these cases do not use these arrays with alignment >1 for compaction.

## Padding Bits (optional)

Using `compactor` without taking care of data padding will produce valid results. But taking care of padding bytes or even padding bits can improve the compression rate. This is because `compactor` searches for matching data in arrays. Since the data content of padding bytes or padding bits can be ignored, the probability of matches increases.

`compactor` handles data structure padding on bit level. For each data byte, there can be an associated padding byte that determines which bit in the data byte contains information (padding bit: 0) and which is meaningless (padding bit: 1). E.g. a padding byte value of 1 indicated that bit 0 in the data byte is a padding bit and bits 1-7 contain information. A value of 128 indicates bit 7 in the data byte is a padding bit, a value of 129 indicates bit 1 and bit 7 are padding bits and so on. A value of 255 indicates that all bits are padding bits, i.e. the data byte is a padding byte an contains no needed information. All padding information for an input data array is given by an array of the same size and is called padding byte mask array. If no padding byte mask array is given all data are considered to be meaningful i.e. no padding bytes/bits are used.

E.g. for color ram values on a Commodore 64 only the lower 4 bits are relevant (= 16 colors), so the upper 4 bits can be specified as padding bits by applying the mask %11110000. For the color "black" (lower 4 bits = %0000) the value 0 can be used, but also 16, 32, 64, 128 and other combinations with the last 4 bits=0. This variety increases the chance for matches with values of other arrays when searching for overlaps.

## Assembler

The input/output format is similar to the ACME crossassembler format. When using ACME, the ideal use case would be:
* Copy read-only array from your assembler file into a compactor input file
* Call `compactor` 
* Include output file into your assembler project

## Input file format

```console
    <array name> <opt: alignment>
    <ascii type> [<value><seperator>]... | <binary type> filename [,[numberBytes] [, [skipBytes]]]
    <opt padding bit mask: <ascii type> [<value><seperator>]... | <binary type> filename [,[numberBytes] [, [skipBytes]]]>
    Empty or commented lines are skipped. Comment token: '/', '#', ':', ';'.

    Format explanation:
    <opt: alignment>  Alignment to compacted array. Default: '1'.
    <ascii type>      Ascii byte type keyword [!byte|!BYTE|!by|!BY|!8|!08].
    <binary type>     Binary byte type keyword [!binary|!bin].
    <value>           Encoding for bytes can be:
                      decimal, e.g. '14' or '-128'.
                      hexadecimal, e.g. '$3d' or '0x02'.
                      binary, e.g. '%0110' or '%10001000'.
                      octal, e.g. '&304'.
                      char, e.g. "C" or '?'. Only charset 1 is supported.
    <seperator>       seperator token ' ', ','.
```
### Array Name and Alignment
Similar to ACME crossassembler syntax with an optional, additional alignment.

**Syntax**<br>
`<array name> <opt: alignment>`<br>
**`array name`**<br>
unique array name<br>
**`alignment`**<br>
array alignment relative to the output array. Default: 1<br>
**Examples**<br>
`sinTable`<br>
`addressTable 2`<br>
`balloonSprite 64`


### ASCII input

Similar to ACME crossassembler syntax.

**Syntax**<br>
`!byte [<value><seperator>]...`<br>
**Alternative notation**<br>
`!byte`, `!BYTE` `!by`, `!BY`, `!8`, `!08`<br>
**`value`**<br>
Encoding for bytes can be<br>
decimal, e.g. '14' or '-128'.<br>
hexadecimal, e.g. '$3d' or '0x02'.<br>
binary, e.g. '%0110' or '%10001000'.<br>
octal, e.g. '&304'.<br>
char, e.g. "C" or '?'. Only charset 1 is supported.<br>
**`seperator`**<br>
seperator token `' ', ','`.<br>
**Examples**<br>
`!byte 0, 255, -1, -128`<br>
`!by 0x0, $ff`<br>
`!08 %c0 %c01 %c11111111`<br>
`!BYTE &0;&01,&377,` <br>
`!BYTE "H", 'E', 'L', 'L', 'O', '!'` <br>

### Binary input

Similar to ACME crossassembler syntax.

**Syntax**<br>
`!binary filename [,[numberBytes] [, [skipBytes]]]`<br>
**Alternative notation**<br>
`!bin`<br>
**`filename`**<br>
binary input filename in quotation marks ""<br>
**`numberBytes`**<br>
read this number of bytes from file. Default: read all bytes from file.<br>
Reading data is stopped after `numberBytes` values<br>
If the file contains less bytes than `numberBytes`, the rest is filled up with value `0`<br>
**`skipBytes`**<br>
Skip this number of bytes from file start (=offset)<br>
If `skipBytes` is `>=` file size and `numberBytes` was set to an certain value, the rest is filled up with value `0`<br>
**Examples**<br>
`!binary "input.bin"` ; read all bytes from binary file "input.bin"<br>
`!bin "sprite.prg", 64, 2` ; insert 64 bytes from file starting with offset 2.<br>
`!bin "address", 2, 10` ; insert 2 bytes from offset 10<br>
`!bin "generated.b",, 22` ; insert all bytes from offset 22 to EOF<br>


## Examples

### ASCII Example

#### Create file `unCompactedData.txt`:

```console
    # comment 1
    firstArray
    !byte 0, 255, -1, -128
    secondArray
    !by 0x0, $ff
    thirdArray
    !08 %0 %01 %11111111
    fourthArray
    !BYTE &0,&01,&377,
    ; comment 2
    fifthArray 4
    !BYTE "H", 'E', 'L', 'L', 'O', '!'
```

#### Call example

```console
    $ compactor -s Size unCompactedData.txt
```

#### Output file example

Output file `compactedData.txt` for call example:
```console
    compactedData
    !byte 8, 5, 12, 12, 15, 33, 0, 255, 255, 128, 0, 1, 255
    firstArray = compactedData + 6
    firstArraySize = 4
    secondArray = compactedData + 6
    secondArraySize = 2
    thirdArray = compactedData + 10
    thirdArraySize = 3
    fourthArray = compactedData + 10
    fourthArraySize = 3
    fifthArray = compactedData + 0
    fifthArraySize = 6
```

### Binary and Data Alignment Example
```console
    arr0
    !binary "data0.bin"          ; numberBytes=read all, skipBytes=0
    arr1 2                       ; alignment=2
    !bin "data1.bin",,1          ; numberBytes=read all, skipBytes=1
    arr2
    !binary "data2.bin",         ; numberBytes=read all, skipBytes=0
    arr3 2                       ; alignment=2
    !binary "data3.bin",,        ; numberBytes=read all, skipBytes=0
    arr4
    !binary "data4.bin",,1       ; numberBytes=read all, skipBytes=1
    arr5 2                       ; alignment=2
    !binary "data5.bin",2,       ; numberBytes=2       , skipBytes=0
    arr6
    !binary "data6.bin",,3       ; numberBytes=read all, skipBytes=3
    arr7 2                       ; alignment=2
    !binary "data7.bin",4,4      ; numberBytes=4       , skipBytes=4
    arr8 
    !binary "data8.bin",4,8      ; numberBytes=4       , skipBytes=8
```

### Padding Bit Mask Examples


```console
    colorRamValues
    !byte 0, 1, 2, 3
    !byte %11110000, %11110000, %11110000, %11110000 ; padding bit mask: bits 0-3 contain information, bits 4-7 are padding bits
    lookupTable
    !byte 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
```

Same example using binary files:

```console
    colorRamValues
    !byte 0, 1, 2, 3
    !binary "colorRamValuesMask.bin"
    lookupTable
    !binary "lookupTableData.bin"
```
<br><br>
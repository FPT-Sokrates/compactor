# `compactor` Dokumentation

## Workflow:

* Create read-only array data description file (input file)
* Call `compactor` with input file
* Use/adapt output file 

## Data Alignment

Data alignment is supported, i.e. given alignments for input arrays are assured relative to the output array. But handle alignments with care: for alignments >1 the resulting compacted array can even be larger than the size of the input arrays. In these cases do not use these arrays with alignment >1 for compaction.

## Assembler

The input/output format is similar to the ACME compiler format. When using ACME, the ideal use case would be:
* Copy read-only array from your assembler file into a compactor input file
* Call `compactor` 
* Include output file into your assembler project

## Input file format

    ```shell
    <array name> <opt: alignment>
    <data type> [<value><seperator>]...
    Empty or commented lines are skipped. Comment token: '/', '#', ':', ';'.

    Format explanation:
    <opt: alignment>                Alignment to compacted array. Default: '1'.
    <data type>                     Byte type keyword [!byte|!BYTE|!by|!BY|!8|!08].
    <value>                         Encoding for bytes can be:
                                    decimal, e.g. '14' or '-128'.
                                    hexadecimal, e.g. '$3d' or '0x02'.
                                    binary, e.g. '%0110' or '%10001000'.
                                    octal, e.g. '&304'.
                                    char, e.g. "C" or '?'.
    <seperator>                     seperator token ' ', ',', ';', ':', '#'.
    ```

## Input file example

Create file `unCompactedData.txt`:
    ```shell
    # comment 1
    firstArray
    !byte 0, 255, -1, -128
    secondArray
    !by 0x0, $ff
    thirdArray
    !08 %0 %01 %11111111
    fourthArray
    !BYTE &0;&01;&377;
    ; comment 2
    fifthArray 4
    !BYTE "H", 'E', 'L', 'L', 'O', '!'
    ```

## Call example

    ```console
    $ compactor -v -s Size unCompactedData.txt
    Original data size:  18
    Compacted data size: 13
    Compaction rate:     0.722222
    ```

## Output file example

Output file `compactedData.txt` for call example:
    ```shell
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


-= EBOOT Signer =-

     by Int-0


  This program is a port of PSCRYPTER by Carlosgs. Code is modified to revert some changes in unpacker/packer routines to make them more flexibles. In PSP version one static buffer was created to store EBOOT.PBP file. In this version, small buffers was created in each I/O operation.

  This program is ported to GNU/Linux (and maybe others) to make signed EBOOT.PBP's with homebrew toolchain. You can install this program into your pspsdk simply doing:

       $ make install

  If you want to make write makefiles with "auto-sign" option add this rule:

EBOOT_signed.PBP: EBOOT.BPB
    ebootsign $^ $@

  Your makefiles now make two files: EBOOT.PBP and EBOOT_signed.PBP (normal homebrew exe and signed exe). If you don't need unsigned file, change the rule:

EBOOT_signed.PBP: EBOOT.PBP
    ebootsign $^ $@
    mv $@ $^
    $(RM) $^

This rule it's a little crap because it don't generate EBOOT_signed.PBP file, buf make signed EBOOT.PBP file. If you have better idea, please send me!

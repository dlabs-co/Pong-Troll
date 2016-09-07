# Pong-Troll
Juego muy troll inspirado en el mítico Pong para Amstrad.<br>
<br>
Instrucciones para Compilación:<br>
sdasz80 -o crt0_cpc.s<br>
sdasz80 -o putchar.s<br>
sdcc -mz80 --fno-omit-frame-pointer --code-loc 0x5038 --data-loc 0 --opt-code-size --no-std-crt0 crt0_cpc.rel putchar.rel PongTroll.c<br>
Para la compilación, es necesario disponer del compilador SDCC.
<br>
Instrucciones para Conversión a DSK:<br>
hex2bin PongTroll.ihx<br>
CPCDiskXP -File PongTroll.bin -AddAmsdosHeader 5000 -AddToNewDsk PongTroll.dsk<br>
<br>
Para la conversión a DSK, son necesarios los programas hex2bin y CPCDiskXP.<br>
Una vez obtenido el .dsk, éste puede cargarse en un emulador como el WinApe, o directamente en un Amstrad CPC con un emulador de disquetera.

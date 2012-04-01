echo ''
echo ''
echo 'Compiling code...'
echo ''
echo 'Starting spu-gcc' 
echo 'spu-gcc spe_code.c -o spe_code'
spu-gcc spe_code.c -o spe_code
echo 'SPU portion completed'
echo ''
echo 'Starting ppu-embedspu'
echo 'ppu-embedspu spe_code spe_code spe_code_csf.o'
ppu-embedspu spe_code spe_code spe_code_csf.o
echo 'Embeding portion completed'
echo ''
echo 'Starting ppu-gcc'
echo 'ppu-gcc ppu_code.c spe_code_csf.o -lspe2 -maltivec -o ppu_code'
ppu-gcc ppu_code.c spe_code_csf.o -lspe2 -o ppu_code -maltivec 
echo ''
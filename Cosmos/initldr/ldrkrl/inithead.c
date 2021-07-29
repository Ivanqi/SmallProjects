#include "cmctl.h"

void inithead_entry()
{
    init_curs();
    close_curs();
    clear_screen(VGADP_DFVL);

    write_realintsvefile();
    write_ldrkrlfile();

    return;
}

// 写initldrsve.bin文件到特定的内存中
void write_realintsvefile()
{

    fhdsc_t *fhdscstart = find_file("initldrsve.bin");
    if (fhdscstart == NULL) {
        error("not file initldrsve.bin");
    }

    m2mcopy((void *)((u32_t)(fhdscstart->fhd_intsfsoff) + LDRFILEADR), (void *)REALDRV_PHYADR, (sint_t)fhdscstart->fhd_frealsz);
    return;
}

/**
 * 在映像文件中查找对应的文件
 * find_file 函数负责扫描映像文件中的文件头描述符，对比其中的文件名
 * 然后返回对应的文件头描述符的地址，这样就可以得到文件在映像文件中的位置和大小了
 */
fhdsc_t *find_file(char_t *fname)
{
    mlosrddsc_t *mrddadrs = MRDDSC_ADR;
    if (mrddadrs->mdc_endgic != MDC_ENDGIC || mrddadrs->mdc_rv != MDC_RVGIC ||
        mrddadrs->mdc_fhdnr < 2 || mrddadrs->mdc_filnr < 2) {
        error("no mrddsc");
    }

    s64_t rethn = -1;
    fhdsc_t *fhdscstart = (fhdsc_t *)((u32_t)(mrddadrs->mdc_fhdbk_s) + LDRFILEADR);

    for (u64_t i = 0; i < mrddadrs->mdc_fhdnr; i++) {
        if (strcmpl(fname, fhdscstart[i].fhd_name) == 0) {
            rethn = (s64_t)i;
            goto ok_l;
        }
    }

    rethn = -1;

ok_l:
    if (rethn < 0) {
        error("not find file");
    }

    return &fhdscstart[rethn];
}

// 写initldrkrl.bin文件到特定的内存中
void write_ldrkrlfile()
{
    fhdsc_t *fhdscstart = find_file("initldrkrl.bin");
    if (fhdscstart == NULL) {
        error("not file initldrkrl.bin");
    }

    m2mcopy((void *)((u32_t)(fhdscstart->fhd_intsfsoff) + LDRFILEADR), (void *)ILDRKRL_PHYADR, (sint_t)fhdscstart->fhd_frealsz);
    
    return ;
}

void error(char_t *estr)
{
    kprint("INITLDR DIE ERROR:%s\n", estr);
    for (;;)
        ;
    
    return;
}

int strcmpl(const char *a, const char *b)
{
    while (*b && *a && (*b == *a)) {
        b++;
        a++;
    }

    return *b - *a;
}

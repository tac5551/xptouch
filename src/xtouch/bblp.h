#ifndef _XLCD_BBLP
#define _XLCD_BBLP

#ifdef __cplusplus
extern "C"
{
#endif

    bool xptouch_bblp_is_p1p()
    {
        return strcmp(xPTouchConfig.xTouchPrinterModel, "C11") == 0;
    }

    bool xptouch_bblp_is_p1s()
    {
        return strcmp(xPTouchConfig.xTouchPrinterModel, "C12") == 0;
    }

    bool xptouch_bblp_is_p1Series()
    {
        return xptouch_bblp_is_p1p() || xptouch_bblp_is_p1s();
    }

    bool xptouch_bblp_is_a1()
    {
        const char *m = xPTouchConfig.xTouchPrinterModel;
        if (!m || !m[0])
            return false;
        if (strcmp(m, "A1") == 0)
            return true;
        return false;
    }

    bool xptouch_bblp_is_a1mini()
    {
        const char *m = xPTouchConfig.xTouchPrinterModel;
        if (!m || !m[0])
            return false;
        if (strcmp(m, "A1 mini") == 0 || strcmp(m, "A1 Mini") == 0 || strcmp(m, "A1Mini") == 0)
            return true;
        return strstr(m, "A1Mini") != NULL;
    }

    bool xptouch_bblp_is_a1Series()
    {
        const char *m = xPTouchConfig.xTouchPrinterModel;
        if (!m || !m[0])
            return false;
        return xptouch_bblp_is_a1() || xptouch_bblp_is_a1mini() || (strstr(m, "A1") != NULL);
    }

    bool xptouch_bblp_is_a1p1Series()
    {
        const char *m = xPTouchConfig.xTouchPrinterModel;
        if (!m || !m[0])
            return false;
        return xptouch_bblp_is_p1Series() || xptouch_bblp_is_a1Series();
    }

    bool xptouch_bblp_is_x1()
    {
        return strcmp(xPTouchConfig.xTouchPrinterModel, "3DPrinter-X1") == 0;
    }

    bool xptouch_bblp_is_x1c()
    {
        return strcmp(xPTouchConfig.xTouchPrinterModel, "3DPrinter-X1-Carbon") == 0;
    }

    bool xptouch_bblp_is_x1Series()
    {
        return xptouch_bblp_is_x1() || xptouch_bblp_is_x1c();
    }

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
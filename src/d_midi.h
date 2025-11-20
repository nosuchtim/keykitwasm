int mdep_getnmidi(char* buff, int buffsize, int* port)
;
void mdep_putnmidi(int n, char *cp, struct Midiport_struct * pport)
;
int openmidiin(int windevno)
;
void mdep_endmidi(void)
;
int mdep_initmidi(struct Midiport_struct *inputs, struct Midiport_struct *outputs)
;
#if KEYCAPTURE
int startvideo(void)
;
void stopvideo(void)
;
#endif
#if KEYDISPLAY
int startdisplay(int noborder, int width, int height)
;
#endif
void setvideogrid(int gx, int gy)
;
Datum mdep_mdep(int argc)
;
#if KEYCAPTURE
#else
#endif
#if KEYDISPLAY
#endif
int openmidiout(int windevno)
;
int mdep_midi(int openclose, struct Midiport_struct * p)
;

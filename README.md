# Krpalnik programske opreme
**Seminar pri predmetu Sistemska programska oprema**

Poročilo in predstavitev seminarja sta dostopna v .pdf in .ppt datotekah.

Programa program.c in patcher.c z demonstracije na seminarju poženemo z ukazi:


```
  gcc program.c                // prevedba v a.out datoteko
  ./a.out                      // zaženemo program, ki naredi izpis
  objdump -d a.out             // za izpis informacij o objektni datoteki (sekcije, simboli, ukazi x86 zbirnega jezika
  gcc -o patcher patcher.c     // prevedemo krpalnik
  ./patcher a.out main         // zaženemo krpalnik s tema argumentoma
  ./b.out                      // zaženemo popravek datoteke a.out, ki je zapisana v b.out)
```

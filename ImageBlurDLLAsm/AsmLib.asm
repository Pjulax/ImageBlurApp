;Tytu³: Filtr rozmycia obrazu
;Autor: Pawe³ Potuczko
;Przedmiot: Jêzyki Asemblerowe
;Rok akademicki: 2020/2021
;Aktualna wersja: 1.0
;Historia:
;v0.1
; - stworzono szkieletowy program asm
;v0.2
; - dodano przechowanie na stosie rejestrów RBP, RBX, RDI, RSP by zachowaæ spójnoœæ pamiêci programu po wykonaniu procedury
; - dodano opis zmiennych jakie s¹ zapisane i w jakich rejestrach
; - dodano tablice mulValue
;v0.3
; - dodano pêtle zagnie¿dzone i iteratory R10 i R11
; - dodano obs³ugê obliczania adresu i dodawania wektorowo wartoœci elementów tablicy
; - skoñczono implementacje algorytmu sk³adaj¹cego wartoœæ jednego punktu w tablicy
; - problem z naruszaniem pamiêci - RDX po jednym przejœciu instrukcji w pêtli nie przechowuje adresu tablicy wyjœciowej
;v0.4
; - przeniesienie na pocz¹tku procedury wartoœci rejetru RDX do rejetru R14
; - debug przetestowany dzia³¹ bez zarzutów
; - release dzia³a tak jakby asemblera nie by³o
;v0.5
; - testy problemów z optymalizacj¹ w konfiguracji
; - przeszukiwanie braków / b³êdów w konfiguracji
;v1.0
; - dodanie brakuj¹cej definicji modu³u .def w linkerze
; - release zacz¹³ dzia³aæ jak nale¿y w ka¿dym ustawieniu w¹tków

.data
mulValue real4 0.0, 0.111, 0.111, 0.111

.code

;Procedura blur_image oblicza rozmycie podanego obrazu bitmapy 24-bitowej, b¹dŸ jego fragmentu, uœredniaj¹c za pomoc¹ œredniej arytmetycznej wartoœci 
;ka¿dego bajtu koloru osobno, wartoœci s¹ obliczane z otaczaj¹cych pikseli w kwadracie 3x3, tablice musz¹ byæ podane
;z indeksami na bajty, a nie na ca³e piksele, dlatego R9 (arrayWidth) przechowuje trzykrotnoœæ szerokoœci obrazu
;obraz wejœciowy jest rozszerzony o nadmiarowy jeden piksel dooko³a wiêc obraz wyjœciowy to (x*3) * y, a obraz wejœciowy to ((x+2)*3) * (y+2)
;obraz wejœciowy powinien zawieraæ w³aœciw¹ czêœæ pikseli na pozycjach odsuniêtych od krawêdzi tablicy conajmniej o 1 piksel,
;piksele otaczaj¹ce obraz to sprawa rozwi¹zania przez u¿ytkownika biblioteki, proponowany sposób, przy wczytaniu obrazu dodaæ do krawêdzi nadmiarowe czarne piksele
;nastêpnie przy ewentualnej fragmentacji podawac obraz z nadmiarowymi liniami z poprzedniego i nastêpnego fragmentu
;@param RCX - QWORD inputPixelArray - wskaŸnik na tablice pikseli - zawiera pikele w formacie BYTE - rozmiar to (arrayHeight+2) * (arrayWidth + 6)
;dodana otoczka z czarnych pikseli i/lub górnej i/lub dolnej linii pikseli obrazu (jeœli to fragment)
;@param RDX - QWORD outputPixelArray - zawiera piksele w formacie BYTE - rozmiar to arrayHeight * arrayWidth
;RDX wejœciowy parametr, który musi zostaæ przesuniêty, tu do R14 - mul nadpisuje zawartoœæ RDX
;@param R8 - UNSIGNED QWORD arrayHeight - wartoœæ maksymalna zewnêtrznej pêtli - wysokoœæ obrazu w pikselach
;@param R9 - UNSIGNED QWORD arrayWidth - wartoœæ maksymalna wewnêtrznej pêtli -
;szerokoœæ obrazu w pikselach pomno¿ona przez 3 sk³adowe piksela (24-bitowy piksel)
;Rejestry wykorzystywane w procedurze: 
;parametry: RCX, RDX, R8, R9,
;zmienne wykorzystywane: RAX, R10, R11, R12, R14, XMM0, XMM1, XMM2, XMM4, RBX (jako jedyny z u¿ywanych jest na pocz¹tku programu zapisywany na stos)
blur_image proc
;@variable R10 - UNSIGNED QWORD - iterator zewnêtrznej pêtli
;@variable R11 - UNSIGNED QWORD - iterator wewnêtrznej pêtli
;@variable R12 - UNSIGNED QWORD - przesuniêcie adresu aktualnego
;@variable R14 - QWORD outputPixelArray - kopia parametru z RDX
;@variable XMM0 - 4 SINGLE PRECISION FLOATS - 3 bajty pikseli z rzêdu 
;@variable XMM1 - 4 SINGLE PRECISION FLOATS - 3 bajty pikseli
;@variable XMM2 - 4 SINGLE PRECISION FLOATS - 3 bajty pikseli
;@variable XMM4 - 4 SINGLE PRECISION FLOATS - 3 sta³e o wartoœci oko³o 1/9
; rejestry RAX i RBX wykorzystywane do obliczeñ adresów

	push rbp ; zapis na stosie rejestru RBP ze wzglêdu na zachowanie spójnoœci pamiêci
    push rbx ; zapis na stosie rejestru RBX ze wzglêdu na zachowanie spójnoœci pamiêci
    push rdi ; zapis na stosie rejestru RDI ze wzglêdu na zachowanie spójnoœci pamiêci
    push rsp ; zapis na stosie rejestru RSP ze wzglêdu na zachowanie spójnoœci pamiêci
    
    xor rax,rax ; zerowanie rejestru RAX
    xor r12,r12 ; zerowanie r12

    mov r14, rdx    ; przesuwam wskaŸnik na tablice z RDX na R14, poniewa¿ RDX jest nadpisywane przy MUL
    xor r10, r10    ; ustawiam iterator zewnêtrzny na 0
    xor r11, r11    ; ustawiam iterator wewnêtrzny na 0
    movaps xmm4, xmmword ptr [mulValue]  ; zapisuje wartoœæ sta³ej 1/9 na ka¿dy fragment w którym mam zamiar trzymaæ dane w rejestrach XMM

@outerLoop:             ;	for (int row = 0; row < arrayHeight; row++) {
    @innerLoop:         ;		for (int column = 0; column < arrayWidth; column++) {
;======= obliczenia offsetu dla punktu w tablicy ===========
        xor rax, rax    ; zerujemy rax
        mov rax, r9     ; pobieramy arrayWidth
        add rax, 6      ; arrayWidth + 6
        mul r10         ; (arrayWidth + 6) * row
        add rax, r11    ; (arrayWidth + 6) * row + column
        mov r12, rax    ; przenosimy offset aktualnego punktu tablicy do R12

;======= pobieram 3 piksele z pierwszej linii ===============
; pix#1
        xorps xmm0, xmm0			    ; zerujemy XMM0 by mieæ pewnoœæ ¿e nic tam nie zosta³o
	    mov al, [rcx + r12]		        ; kopiujemy do AL wartoœæ bajta z tablicy wejœciowej
	    movzx eax, al				    ; wykonujemy zero extend - rozszerzamy 8 bitow¹ wartoœæ do 32 bitowej
	    cvtsi2ss xmm0, eax              ; podajemy wartoœæ z EAX z konwersj¹ na Single precision float do XMM0
        pshufd xmm0, xmm0, 00100100b    ; pix#1 0 0 pix#1 - rozmieszczamy elementy XMM0 wed³ug maski
; pix#2
        xor rbx, rbx                    ; zerujemy RBX
        mov rbx, r12                    ; kopiujemy przesuniêcie komórki pocz¹tkowej
        add rbx, 3                      ; dodajemy przesuniêcie o piksel do kolejnej komórki
        mov al, [rcx + rbx]             ; kopiujemy do AL wartoœæ bajta z tablicy wejœciowej
        movzx eax, al                   ; wykonujemy zero extend - rozszerzamy 8 bitow¹ wartoœæ do 32 bitowej
        cvtsi2ss xmm0, eax              ; podajemy wartoœæ z EAX z konwersj¹ na Single precision float do XMM0
        pshufd xmm0, xmm0, 11000100b    ; pix#1 pix#2 0 pix#2 - rozmieszczamy elementy XMM0 wed³ug maski
; pix#3
        mov rbx, r12                    ; kopiujemy przesuniêcie komórki pocz¹tkowej
        add rbx, 3                      ; dodajemy przesuniêcie o piksel do kolejnej komórki
        mov al, [rcx + rbx]             ; kopiujemy do AL wartoœæ bajta z tablicy wejœciowej
        movzx eax, al                   ; wykonujemy zero extend - rozszerzamy 8 bitow¹ wartoœæ do 32 bitowej
        cvtsi2ss xmm0, eax              ; podajemy wartoœæ z EAX z konwersj¹ na Single precision float do XMM0
        pshufd xmm0, xmm0, 11100001b    ; pix#1 pix#2 pix#3 0 - rozmieszczamy elementy XMM0 wed³ug maski
        
;======= pobieram 3 piksele z drugiej linii ===============
; pix#4
        xor rbx, rbx                    ; zerujemy RBX
        mov rbx, r12                    ; kopiujemy przesuniêcie komórki pocz¹tkowej
        add rbx, r9                     ; dodajemy przesuniêcie o wiersz obrazu
        add rbx, 6                      ; i o boczne piksele uzupe³niaj¹ce
        xorps xmm1, xmm1			    ; zerujemy XMM1 by mieæ pewnoœæ ¿e nic tam nie zosta³o
	    mov al, [rcx + rbx]		        ; kopiujemy do AL wartoœæ bajta z tablicy wejœciowej
	    movzx eax, al				    ; wykonujemy zero extend - rozszerzamy 8 bitow¹ wartoœæ do 32 bitowej
	    cvtsi2ss xmm1, eax              ; podajemy wartoœæ z EAX z konwersj¹ na Single precision float do XMM1
        pshufd xmm1, xmm1, 00100100b    ; pix#4 0 0 pix#4 - rozmieszczamy elementy XMM1 wed³ug maski
; pix#5
        mov rbx, r12                    ; kopiujemy przesuniêcie komórki pocz¹tkowej
        add rbx, r9                     ; dodajemy przesuniêcie o wiersz obrazu
        add rbx, 6                      ; dodajemy przesuniêcie o piksel do kolejnej komórki
        mov al, [rcx + rbx]             ; kopiujemy do AL wartoœæ bajta z tablicy wejœciowej
        movzx eax, al                   ; wykonujemy zero extend - rozszerzamy 8 bitow¹ wartoœæ do 32 bitowej
        cvtsi2ss xmm1, eax              ; podajemy wartoœæ z EAX z konwersj¹ na Single precision float do XMM1
        pshufd xmm1, xmm1, 11000100b    ; pix#4 pix#5 0 pix#5 - rozmieszczamy elementy XMM1 wed³ug maski
; pix#6
        mov rbx, r12                    ; kopiujemy przesuniêcie komórki pocz¹tkowej
        add rbx, r9                     ; dodajemy przesuniêcie o wiersz obrazu
        add rbx, 6                      ; dodajemy przesuniêcie o piksel do kolejnej komórki
        mov al, [rcx + rbx]             ; kopiujemy do AL wartoœæ bajta z tablicy wejœciowej
        movzx eax, al                   ; wykonujemy zero extend - rozszerzamy 8 bitow¹ wartoœæ do 32 bitowej
        cvtsi2ss xmm1, eax              ; podajemy wartoœæ z EAX z konwersj¹ na Single precision float do XMM1
        pshufd xmm1, xmm1, 11100001b    ; pix#4 pix#5 pix#6 0 - rozmieszczamy elementy XMM1 wed³ug maski

;======= pobieram 3 piksele z trzeciej linii ===============
; pix#7
        xor rbx, rbx                    ; zerujemy RBX
        mov rbx, r12                    ; kopiujemy przesuniêcie komórki pocz¹tkowej
        add rbx, r9                     ; dodajemy przesuniêcie o wiersz obrazu
        add rbx, r9                     ; dodajemy przesuniêcie o drugi wiersz obrazu
        add rbx, 12                     ; i o boczne piksele uzupe³niaj¹ce
        xorps xmm2, xmm2			    ; zerujemy XMM2 by mieæ pewnoœæ ¿e nic tam nie zosta³o
	    mov al, [rcx + rbx]		        ; kopiujemy do AL wartoœæ bajta z tablicy wejœciowej
	    movzx eax, al				    ; wykonujemy zero extend - rozszerzamy 8 bitow¹ wartoœæ do 32 bitowej
	    cvtsi2ss xmm2, eax              ; podajemy wartoœæ z EAX z konwersj¹ na Single precision float do XMM2
        pshufd xmm2, xmm2, 00100100b    ; pix#7 0 0 pix#7 - rozmieszczamy elementy XMM2 wed³ug maski
; pix#8
        mov rbx, r12                    ; kopiujemy przesuniêcie komórki pocz¹tkowej
        add rbx, r9                     ; dodajemy przesuniêcie o wiersz obrazu
        add rbx, r9                     ; dodajemy przesuniêcie o drugi wiersz obrazu
        add rbx, 12                      ; dodajemy przesuniêcie o piksel do kolejnej komórki
        mov al, [rcx + rbx]             ; kopiujemy do AL wartoœæ bajta z tablicy wejœciowej
        movzx eax, al                   ; wykonujemy zero extend - rozszerzamy 8 bitow¹ wartoœæ do 32 bitowej
        cvtsi2ss xmm2, eax              ; podajemy wartoœæ z EAX z konwersj¹ na Single precision float do XMM2
        pshufd xmm2, xmm2, 11000100b    ; pix#7 pix#8 0 pix#8 - rozmieszczamy elementy XMM2 wed³ug maski
; pix#9
        mov rbx, r12                    ; kopiujemy przesuniêcie komórki pocz¹tkowej
        add rbx, r9                     ; dodajemy przesuniêcie o wiersz obrazu
        add rbx, r9                     ; dodajemy przesuniêcie o drugi wiersz obrazu
        add rbx, 12                      ; dodajemy przesuniêcie o piksel do kolejnej komórki
        mov al, [rcx + rbx]             ; kopiujemy do AL wartoœæ bajta z tablicy wejœciowej
        movzx eax, al                   ; wykonujemy zero extend - rozszerzamy 8 bitow¹ wartoœæ do 32 bitowej
        cvtsi2ss xmm2, eax              ; podajemy wartoœæ z EAX z konwersj¹ na Single precision float do XMM2
        pshufd xmm2, xmm2, 11100001b    ; pix#7 pix#8 pix#9 0 - rozmieszczamy elementy XMM2 wed³ug maski

;============== obliczenia wartosci piksela ===============
        mulps xmm0, xmm4    ; mno¿ymy punkty w tablicy razy sta³¹ 0.111
        mulps xmm1, xmm4
        mulps xmm2, xmm4
        addps xmm0, xmm1    ; dodajemy do siebie wektory
        addps xmm0, xmm2
        haddps xmm0, xmm0   ; wykorzystujemy dodawanie horyzontalne dwukrotnie - ³¹cznie to dodanie 4 zmiennych w jedn¹
        haddps xmm0, xmm0   ;[127:96] + [95:64] prawego argumentu id¹ do [127:96] czêœci lewego argumentu 
                            ;[63:32] + [31:0] prawego argumentu id¹ do [95:64] czêœci lewego argumentu
                            ;[127:96] + [95:64] lewego argumentu id¹ do [63:32] czêœci lewego argumentu
                            ;[63:32] + [31:0] lewego argumentu id¹ do [31:0] czêœci lewego argumentu

        xor rbx, rbx        ; obliczam offset dla czêœci piksela w wyjœciowej tablicy
        xor rax, rax
        mov rax, r9
        mul r10
        add rax, r11
        mov rbx, rax

        xor eax, eax
        cvtss2si eax, xmm0  ; konwersja single precision float na dword integer
        mov [r14 + rbx], al ; zapisujemy bajt z przekonwertowanego wy¿ej elementu do tabeli wyjœciowej na wyliczony wczeœniej adres

;============= inkrementacja i sprawdzanie warunku @innerLoop ============
        inc r11         ; inkrementuje iterator wewnêtrzny
        cmp r11, r9     ; sprwadzam czy wewnêtrzna pêtla nie osi¹gnê³a krañca
        jb @innerLoop   ; jeœli nie, zacznij kolejn¹ iteracjê
        
;============= inkrementacja i sprawdzanie warunku @outerLoop ============
    mov r11, 0          ; zeruje wewnêtrzny iterator
    inc r10             ; inkrementuje zewnêtrzny iterator
    cmp r10, r8         ; sprawdzam czy nie przekroczy³ krañcowej wartoœci
    jb @outerLoop       ; jeœli nie, zacznij kolejn¹ iteracjê
                        ; jeœli przekroczy³, zakoñcz procedurê

	pop rsp ; pobranie ze stosu wartoœci rejestru RSP ze wzglêdu na zachowanie spójnoœci pamiêci
	pop rdi ; pobranie ze stosu wartoœci rejestru RDI ze wzglêdu na zachowanie spójnoœci pamiêci
    pop rbx ; pobranie ze stosu wartoœci rejestru RBX ze wzglêdu na zachowanie spójnoœci pamiêci
    pop rbp ; pobranie ze stosu wartoœci rejestru RBP ze wzglêdu na zachowanie spójnoœci pamiêci
    xor eax, eax ; wartoœæ zwrotna 0
	ret

blur_image endp
end
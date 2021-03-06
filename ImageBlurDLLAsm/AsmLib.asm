;Tytu�: Filtr rozmycia obrazu
;Autor: Pawe� Potuczko
;Przedmiot: J�zyki Asemblerowe
;Rok akademicki: 2020/2021
;Aktualna wersja: 1.0
;Historia:
;v0.1
; - stworzono szkieletowy program asm
;v0.2
; - dodano przechowanie na stosie rejestr�w RBP, RBX, RDI, RSP by zachowa� sp�jno�� pami�ci programu po wykonaniu procedury
; - dodano opis zmiennych jakie s� zapisane i w jakich rejestrach
; - dodano tablice mulValue
;v0.3
; - dodano p�tle zagnie�dzone i iteratory R10 i R11
; - dodano obs�ug� obliczania adresu i dodawania wektorowo warto�ci element�w tablicy
; - sko�czono implementacje algorytmu sk�adaj�cego warto�� jednego punktu w tablicy
; - problem z naruszaniem pami�ci - RDX po jednym przej�ciu instrukcji w p�tli nie przechowuje adresu tablicy wyj�ciowej
;v0.4
; - przeniesienie na pocz�tku procedury warto�ci rejetru RDX do rejetru R14
; - debug przetestowany dzia�� bez zarzut�w
; - release dzia�a tak jakby asemblera nie by�o
;v0.5
; - testy problem�w z optymalizacj� w konfiguracji
; - przeszukiwanie brak�w / b��d�w w konfiguracji
;v1.0
; - dodanie brakuj�cej definicji modu�u .def w linkerze
; - release zacz�� dzia�a� jak nale�y w ka�dym ustawieniu w�tk�w

.data
mulValue real4 0.0, 0.111, 0.111, 0.111

.code

;Procedura blur_image oblicza rozmycie podanego obrazu bitmapy 24-bitowej, b�d� jego fragmentu, u�redniaj�c za pomoc� �redniej arytmetycznej warto�ci 
;ka�dego bajtu koloru osobno, warto�ci s� obliczane z otaczaj�cych pikseli w kwadracie 3x3, tablice musz� by� podane
;z indeksami na bajty, a nie na ca�e piksele, dlatego R9 (arrayWidth) przechowuje trzykrotno�� szeroko�ci obrazu
;obraz wej�ciowy jest rozszerzony o nadmiarowy jeden piksel dooko�a wi�c obraz wyj�ciowy to (x*3) * y, a obraz wej�ciowy to ((x+2)*3) * (y+2)
;obraz wej�ciowy powinien zawiera� w�a�ciw� cz�� pikseli na pozycjach odsuni�tych od kraw�dzi tablicy conajmniej o 1 piksel,
;piksele otaczaj�ce obraz to sprawa rozwi�zania przez u�ytkownika biblioteki, proponowany spos�b, przy wczytaniu obrazu doda� do kraw�dzi nadmiarowe czarne piksele
;nast�pnie przy ewentualnej fragmentacji podawac obraz z nadmiarowymi liniami z poprzedniego i nast�pnego fragmentu
;@param RCX - QWORD inputPixelArray - wska�nik na tablice pikseli - zawiera pikele w formacie BYTE - rozmiar to (arrayHeight+2) * (arrayWidth + 6)
;dodana otoczka z czarnych pikseli i/lub g�rnej i/lub dolnej linii pikseli obrazu (je�li to fragment)
;@param RDX - QWORD outputPixelArray - zawiera piksele w formacie BYTE - rozmiar to arrayHeight * arrayWidth
;RDX wej�ciowy parametr, kt�ry musi zosta� przesuni�ty, tu do R14 - mul nadpisuje zawarto�� RDX
;@param R8 - UNSIGNED QWORD arrayHeight - warto�� maksymalna zewn�trznej p�tli - wysoko�� obrazu w pikselach
;@param R9 - UNSIGNED QWORD arrayWidth - warto�� maksymalna wewn�trznej p�tli -
;szeroko�� obrazu w pikselach pomno�ona przez 3 sk�adowe piksela (24-bitowy piksel)
;Rejestry wykorzystywane w procedurze: 
;parametry: RCX, RDX, R8, R9,
;zmienne wykorzystywane: RAX, R10, R11, R12, R14, XMM0, XMM1, XMM2, XMM4, RBX (jako jedyny z u�ywanych jest na pocz�tku programu zapisywany na stos)
blur_image proc
;@variable R10 - UNSIGNED QWORD - iterator zewn�trznej p�tli
;@variable R11 - UNSIGNED QWORD - iterator wewn�trznej p�tli
;@variable R12 - UNSIGNED QWORD - przesuni�cie adresu aktualnego
;@variable R14 - QWORD outputPixelArray - kopia parametru z RDX
;@variable XMM0 - 4 SINGLE PRECISION FLOATS - 3 bajty pikseli z rz�du 
;@variable XMM1 - 4 SINGLE PRECISION FLOATS - 3 bajty pikseli
;@variable XMM2 - 4 SINGLE PRECISION FLOATS - 3 bajty pikseli
;@variable XMM4 - 4 SINGLE PRECISION FLOATS - 3 sta�e o warto�ci oko�o 1/9
; rejestry RAX i RBX wykorzystywane do oblicze� adres�w

	push rbp ; zapis na stosie rejestru RBP ze wzgl�du na zachowanie sp�jno�ci pami�ci
    push rbx ; zapis na stosie rejestru RBX ze wzgl�du na zachowanie sp�jno�ci pami�ci
    push rdi ; zapis na stosie rejestru RDI ze wzgl�du na zachowanie sp�jno�ci pami�ci
    push rsp ; zapis na stosie rejestru RSP ze wzgl�du na zachowanie sp�jno�ci pami�ci
    
    xor rax,rax ; zerowanie rejestru RAX
    xor r12,r12 ; zerowanie r12

    mov r14, rdx    ; przesuwam wska�nik na tablice z RDX na R14, poniewa� RDX jest nadpisywane przy MUL
    xor r10, r10    ; ustawiam iterator zewn�trzny na 0
    xor r11, r11    ; ustawiam iterator wewn�trzny na 0
    movaps xmm4, xmmword ptr [mulValue]  ; zapisuje warto�� sta�ej 1/9 na ka�dy fragment w kt�rym mam zamiar trzyma� dane w rejestrach XMM

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
        xorps xmm0, xmm0			    ; zerujemy XMM0 by mie� pewno�� �e nic tam nie zosta�o
	    mov al, [rcx + r12]		        ; kopiujemy do AL warto�� bajta z tablicy wej�ciowej
	    movzx eax, al				    ; wykonujemy zero extend - rozszerzamy 8 bitow� warto�� do 32 bitowej
	    cvtsi2ss xmm0, eax              ; podajemy warto�� z EAX z konwersj� na Single precision float do XMM0
        pshufd xmm0, xmm0, 00100100b    ; pix#1 0 0 pix#1 - rozmieszczamy elementy XMM0 wed�ug maski
; pix#2
        xor rbx, rbx                    ; zerujemy RBX
        mov rbx, r12                    ; kopiujemy przesuni�cie kom�rki pocz�tkowej
        add rbx, 3                      ; dodajemy przesuni�cie o piksel do kolejnej kom�rki
        mov al, [rcx + rbx]             ; kopiujemy do AL warto�� bajta z tablicy wej�ciowej
        movzx eax, al                   ; wykonujemy zero extend - rozszerzamy 8 bitow� warto�� do 32 bitowej
        cvtsi2ss xmm0, eax              ; podajemy warto�� z EAX z konwersj� na Single precision float do XMM0
        pshufd xmm0, xmm0, 11000100b    ; pix#1 pix#2 0 pix#2 - rozmieszczamy elementy XMM0 wed�ug maski
; pix#3
        mov rbx, r12                    ; kopiujemy przesuni�cie kom�rki pocz�tkowej
        add rbx, 3                      ; dodajemy przesuni�cie o piksel do kolejnej kom�rki
        mov al, [rcx + rbx]             ; kopiujemy do AL warto�� bajta z tablicy wej�ciowej
        movzx eax, al                   ; wykonujemy zero extend - rozszerzamy 8 bitow� warto�� do 32 bitowej
        cvtsi2ss xmm0, eax              ; podajemy warto�� z EAX z konwersj� na Single precision float do XMM0
        pshufd xmm0, xmm0, 11100001b    ; pix#1 pix#2 pix#3 0 - rozmieszczamy elementy XMM0 wed�ug maski
        
;======= pobieram 3 piksele z drugiej linii ===============
; pix#4
        xor rbx, rbx                    ; zerujemy RBX
        mov rbx, r12                    ; kopiujemy przesuni�cie kom�rki pocz�tkowej
        add rbx, r9                     ; dodajemy przesuni�cie o wiersz obrazu
        add rbx, 6                      ; i o boczne piksele uzupe�niaj�ce
        xorps xmm1, xmm1			    ; zerujemy XMM1 by mie� pewno�� �e nic tam nie zosta�o
	    mov al, [rcx + rbx]		        ; kopiujemy do AL warto�� bajta z tablicy wej�ciowej
	    movzx eax, al				    ; wykonujemy zero extend - rozszerzamy 8 bitow� warto�� do 32 bitowej
	    cvtsi2ss xmm1, eax              ; podajemy warto�� z EAX z konwersj� na Single precision float do XMM1
        pshufd xmm1, xmm1, 00100100b    ; pix#4 0 0 pix#4 - rozmieszczamy elementy XMM1 wed�ug maski
; pix#5
        mov rbx, r12                    ; kopiujemy przesuni�cie kom�rki pocz�tkowej
        add rbx, r9                     ; dodajemy przesuni�cie o wiersz obrazu
        add rbx, 6                      ; dodajemy przesuni�cie o piksel do kolejnej kom�rki
        mov al, [rcx + rbx]             ; kopiujemy do AL warto�� bajta z tablicy wej�ciowej
        movzx eax, al                   ; wykonujemy zero extend - rozszerzamy 8 bitow� warto�� do 32 bitowej
        cvtsi2ss xmm1, eax              ; podajemy warto�� z EAX z konwersj� na Single precision float do XMM1
        pshufd xmm1, xmm1, 11000100b    ; pix#4 pix#5 0 pix#5 - rozmieszczamy elementy XMM1 wed�ug maski
; pix#6
        mov rbx, r12                    ; kopiujemy przesuni�cie kom�rki pocz�tkowej
        add rbx, r9                     ; dodajemy przesuni�cie o wiersz obrazu
        add rbx, 6                      ; dodajemy przesuni�cie o piksel do kolejnej kom�rki
        mov al, [rcx + rbx]             ; kopiujemy do AL warto�� bajta z tablicy wej�ciowej
        movzx eax, al                   ; wykonujemy zero extend - rozszerzamy 8 bitow� warto�� do 32 bitowej
        cvtsi2ss xmm1, eax              ; podajemy warto�� z EAX z konwersj� na Single precision float do XMM1
        pshufd xmm1, xmm1, 11100001b    ; pix#4 pix#5 pix#6 0 - rozmieszczamy elementy XMM1 wed�ug maski

;======= pobieram 3 piksele z trzeciej linii ===============
; pix#7
        xor rbx, rbx                    ; zerujemy RBX
        mov rbx, r12                    ; kopiujemy przesuni�cie kom�rki pocz�tkowej
        add rbx, r9                     ; dodajemy przesuni�cie o wiersz obrazu
        add rbx, r9                     ; dodajemy przesuni�cie o drugi wiersz obrazu
        add rbx, 12                     ; i o boczne piksele uzupe�niaj�ce
        xorps xmm2, xmm2			    ; zerujemy XMM2 by mie� pewno�� �e nic tam nie zosta�o
	    mov al, [rcx + rbx]		        ; kopiujemy do AL warto�� bajta z tablicy wej�ciowej
	    movzx eax, al				    ; wykonujemy zero extend - rozszerzamy 8 bitow� warto�� do 32 bitowej
	    cvtsi2ss xmm2, eax              ; podajemy warto�� z EAX z konwersj� na Single precision float do XMM2
        pshufd xmm2, xmm2, 00100100b    ; pix#7 0 0 pix#7 - rozmieszczamy elementy XMM2 wed�ug maski
; pix#8
        mov rbx, r12                    ; kopiujemy przesuni�cie kom�rki pocz�tkowej
        add rbx, r9                     ; dodajemy przesuni�cie o wiersz obrazu
        add rbx, r9                     ; dodajemy przesuni�cie o drugi wiersz obrazu
        add rbx, 12                      ; dodajemy przesuni�cie o piksel do kolejnej kom�rki
        mov al, [rcx + rbx]             ; kopiujemy do AL warto�� bajta z tablicy wej�ciowej
        movzx eax, al                   ; wykonujemy zero extend - rozszerzamy 8 bitow� warto�� do 32 bitowej
        cvtsi2ss xmm2, eax              ; podajemy warto�� z EAX z konwersj� na Single precision float do XMM2
        pshufd xmm2, xmm2, 11000100b    ; pix#7 pix#8 0 pix#8 - rozmieszczamy elementy XMM2 wed�ug maski
; pix#9
        mov rbx, r12                    ; kopiujemy przesuni�cie kom�rki pocz�tkowej
        add rbx, r9                     ; dodajemy przesuni�cie o wiersz obrazu
        add rbx, r9                     ; dodajemy przesuni�cie o drugi wiersz obrazu
        add rbx, 12                      ; dodajemy przesuni�cie o piksel do kolejnej kom�rki
        mov al, [rcx + rbx]             ; kopiujemy do AL warto�� bajta z tablicy wej�ciowej
        movzx eax, al                   ; wykonujemy zero extend - rozszerzamy 8 bitow� warto�� do 32 bitowej
        cvtsi2ss xmm2, eax              ; podajemy warto�� z EAX z konwersj� na Single precision float do XMM2
        pshufd xmm2, xmm2, 11100001b    ; pix#7 pix#8 pix#9 0 - rozmieszczamy elementy XMM2 wed�ug maski

;============== obliczenia wartosci piksela ===============
        mulps xmm0, xmm4    ; mno�ymy punkty w tablicy razy sta�� 0.111
        mulps xmm1, xmm4
        mulps xmm2, xmm4
        addps xmm0, xmm1    ; dodajemy do siebie wektory
        addps xmm0, xmm2
        haddps xmm0, xmm0   ; wykorzystujemy dodawanie horyzontalne dwukrotnie - ��cznie to dodanie 4 zmiennych w jedn�
        haddps xmm0, xmm0   ;[127:96] + [95:64] prawego argumentu id� do [127:96] cz�ci lewego argumentu 
                            ;[63:32] + [31:0] prawego argumentu id� do [95:64] cz�ci lewego argumentu
                            ;[127:96] + [95:64] lewego argumentu id� do [63:32] cz�ci lewego argumentu
                            ;[63:32] + [31:0] lewego argumentu id� do [31:0] cz�ci lewego argumentu

        xor rbx, rbx        ; obliczam offset dla cz�ci piksela w wyj�ciowej tablicy
        xor rax, rax
        mov rax, r9
        mul r10
        add rax, r11
        mov rbx, rax

        xor eax, eax
        cvtss2si eax, xmm0  ; konwersja single precision float na dword integer
        mov [r14 + rbx], al ; zapisujemy bajt z przekonwertowanego wy�ej elementu do tabeli wyj�ciowej na wyliczony wcze�niej adres

;============= inkrementacja i sprawdzanie warunku @innerLoop ============
        inc r11         ; inkrementuje iterator wewn�trzny
        cmp r11, r9     ; sprwadzam czy wewn�trzna p�tla nie osi�gn�a kra�ca
        jb @innerLoop   ; je�li nie, zacznij kolejn� iteracj�
        
;============= inkrementacja i sprawdzanie warunku @outerLoop ============
    mov r11, 0          ; zeruje wewn�trzny iterator
    inc r10             ; inkrementuje zewn�trzny iterator
    cmp r10, r8         ; sprawdzam czy nie przekroczy� kra�cowej warto�ci
    jb @outerLoop       ; je�li nie, zacznij kolejn� iteracj�
                        ; je�li przekroczy�, zako�cz procedur�

	pop rsp ; pobranie ze stosu warto�ci rejestru RSP ze wzgl�du na zachowanie sp�jno�ci pami�ci
	pop rdi ; pobranie ze stosu warto�ci rejestru RDI ze wzgl�du na zachowanie sp�jno�ci pami�ci
    pop rbx ; pobranie ze stosu warto�ci rejestru RBX ze wzgl�du na zachowanie sp�jno�ci pami�ci
    pop rbp ; pobranie ze stosu warto�ci rejestru RBP ze wzgl�du na zachowanie sp�jno�ci pami�ci
    xor eax, eax ; warto�� zwrotna 0
	ret

blur_image endp
end
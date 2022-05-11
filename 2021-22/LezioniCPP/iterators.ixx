
export module iterators;

import <iostream>;
import <vector>;
import <list>;
import <string>;
import pairs;
import smart_ptr;

using namespace std;


// questa funzione � templatizzata su un tipo Container QUALUNQUE
// non c'� alcun genere di specifica dei requisiti che questo tipo Container deve rispettare: il nome stesso � totalmente arbitrario
// l'implementazione fa emergere la necessit� che questo tipo Container risponda a certi requisiti
template <class Container>
void increment_all(Container& v)
{
	// la keyword typename indica al compilatore che l'identificatore che segue � un TIPO e non un membro statico
	// senza typename il compilatore potrebbe confondere Container::iterator con un campo statico di nome iterator dentro la classe Container
	for (typename Container::iterator it = v.begin(); it != v.end(); ++it)	// richiede l'esistenza di un member type di nome iterator e dei metodi begin() ed end()
	{																		// inoltre richiede l'operatore di post-incremento sul tipo dell'iteratore
		typename Container::reference v = *it;	// richiede l'esistenza di un member type di nome reference (di solito � equivalente a value_type&)
		++v;									// richiede l'esistenza dell'operatore di pre-incremento per il tipo dell'elemento del container
	}
}

template <class Container>
void print_all(const Container& v)
{
	cout << typeid(Container).name() << "[";
	for (typename Container::const_iterator it = v.begin(); it != v.end(); ++it)	// usiamo il const_iterator perch� non dobbiamo modificare gli elementi del container
	{
		cout << *it << ", ";	// richiede l'operatore << sul tipo dell'elemento del container
	}
	cout << "\b\b]" << endl;
}

using pairs::mypair;

// definiamo gli operatori di output stream per smart_ptr e mypair
template <class A, class B>
ostream& operator<<(ostream& os, const mypair<A, B>& p)
{
	return os << "(" << p.fst() << ", " << p.snd() << ")";
}


// ridefinisce l'operatore GLOBALE di streaming per il tipo double* stampando sia l'indirizzo che il double puntato
// ATTENZIONE: la sintassi col 'const&' DOPO il tipo � una sintassi alternativa: scrivere 'const MioTipo&' e 'MioTipo const&' � esattamente EQUIVALENTE ed indica un const-reference di tipo MioTipo.
//   C++ supporta dal vanilla sia la sintassi PREFISSA con il const PRIMA del tipo, sia quella SUFFISSA con il const DOPO il tipo.
//   Nel nostro caso per� MioTipo � un pointer a double e qui emerge un problema sottile: il const PREFISSO si riferisce al tipo PUNTATO, non al puntatore; ma come posso specificare che voglio prendere il PUNTATOTORE per const-reference?
//   In altre parole, voglio un const-reference ad un puntatore che punta ad un const double: per farlo devo mescolare la sintassi prefissa con quella suffissa e scrivere 'const double* const&'.
//   Il motivo per cui voglio che sia il puntatore sia il puntato siano const � perch� sto ridefinendo l'operator<< GLOBALE per i double*, quindi devo riprodurre la firma ESATTA originale per assicurarmi di non fare un overload per sbaglio.
//   La firma esatta originale �:
//	     ostream& operator<<(ostream& os, double* p)
// 
//   Quanti e quali altri modi di scrivere il tipo del parametro p esistono e sono equivalenti ad un semplice pointer a double per valore?
//       double* const&				prendo per const-reference (suffisso) un pointer ad un double non-const
//       const double* const&		prendo per const-reference (suffisso) un pointer ad un double const (prefisso)
//       double* const              prendo per copia un pointer const (suffisso) ad un double non-const
//       const double* const        prendo per copia un pointer const (suffisso) ad un double const (prefisso)
//       double*                    prendo per copia un pointer non-const ad un double non-const
//       const double*              prendo per copia un pointer non-const ad un double const (prefisso)
// 
//   E ora facciamoci la domanda inversa: quali tipi sono invece diversi e farebbero un overload anzich� uno shadowing?
//       const double*&				prendo per reference un pointer non-const ad un double const (prefisso)
//       double*&					prendo per reference un pointer non-const ad un double non-const
//   
//   Per essere equivalenti alla firma originale (double*) non importa se il double � const oppure no, quello che importa � che il pointer sia const se lo prendiamo per reference, perch� se non � una copia non deve essere modificabile.
//   Il significato di tutto questo �: prendere un argomento per valore (cio� per copia) � compatibile, dal punto di vista dei tipi del compilatore, con il prendere per const-reference.
//   Non perch� sia la stessa cosa (perch� sappiamo che sono cose diverse) ma perch� dal punto di vista della soundness � uguale: un argomento passato per copia garantisce la non modificabilit� dell'originale, esattamente come lo garantisce il prenderlo per const-reference.
//   
//   Nel nostro caso usiamo il tipo pi� sicuro tra quelli equivalenti a double*, cio� const double* const&
//
// ESPERIMENTO: provate a cambiare il tipo del parametro p in const double*& ad esempio, e lanciate il programma di nuovo: vedrete che il printing dei double* sar� quello originale e non quello definito da noi, perch� quella qui sotto diventa un overload, non uno shadowing di quella globale.
ostream& operator<<(ostream& os, const double* const& p)
{
	os << "&" << static_cast<const void*>(p);			// castiamo a const void* per stampare l'indirizzo numerico invocando l'operator<< sui void* definito globalmente dalla standard library
														// lo static_cast non ha il potere di rimuovere un const, quindi non si pu� castare a void* semplicemente
														// ATTENZIONE: senza cast diventerebbe una chiamata RICORSIVA all'operator<< che noi stessi stiamo definendo
	if (p != nullptr) os << "[" << *p << "]";			// se il pointer non � nullo printa anche il dereference
	return os;
}

export void test_iterators()
{
	vector<int> v{ 1, 2, 3, 4 };
	print_all(v);		// questa chiamata genera una istanza della funzione print_all() con vector<int> sostituo al template parameter Container
	increment_all(v);	// questa chiamata genera una istanza della funzione print_all() con vector<int> sostituo al template parameter Container
	print_all(v);		// questa NON rigenera un'altra istanza della funzione print_all() per vector<int>: per ogni tipo diverso ne viene generata UNA SOLA

	list<string> l1{ "ciao", "sono", "io" };
	print_all(l1);			// questa chiamata genera una istanza di print_all() per list<string>
	//increment_all(l1);	// NON COMPILA: questa chiamata genera una istanza di increment_all() per list<string>, ma il pre-incremento non esiste per string

	double arr[] = { 11.23, 35.53 };
	list<mypair<unsigned int, double*>> l2{ {1u, arr}, {2u, arr}, {3u, arr} };
	print_all(l2);		// questa genera una istanza della print_all() per list<mypair<unsigned int, double*>>
						// essa compila perch� abbiamo definito l'operator<< per mypair, altrimenti non compilerebbe
	increment_all(l2);	// questa compila perch� mypair ha l'operatore di pre-incremento, il quale invoca a sua volta il pre-incremento sui due campi della coppia
						// pertanto la chiamata compila perch� sia unsigned int che double* supportano il pre-incremento
	print_all(l2);
}


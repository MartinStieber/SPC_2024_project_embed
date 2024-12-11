/** \file TQueue.cpp
 *  \brief Implementace API pro typ fronta (realizace pomocí kruhového pole)
 *  \author Petyovský
 *  \version 2022
 *  $Id: TQueue.c 1611 2022-03-01 15:12:26Z petyovsky $
 */

#include "TQueue.h"
#include <assert.h>

void queue_init(struct TQueue *aQueue)
{
	// Prvotní nastavení vnitřních proměnných fronty.
	// Pokud parametr typu ukazatel na TQueue není NULL,
	// vynuluj indexy do pole pro čelo (iPopPos) i konec (iPushPos) fronty.
	if (aQueue)
	{
		aQueue->iPopPos = 0;
		aQueue->iPushPos = 0;
	}
}

char queue_is_empty(const struct TQueue *aQueue)
{
	// Test, zda je fronta prázdná - (tj. fronta neobsahuje žádné elementy).
	// Pokud parametr typu ukazatel na TQueue není NULL,
	// assercí otestuj konzistenci indexů na počátek a konec fronty (oba indexy musí být menší než QUEUE_MAXCOUNT),
	// odkazují-li indexy do pole pro čelo i konec fronty na stejný element vrať 1,
	// jinak vrať 0.
	if (aQueue)
	{
		assert((aQueue->iPopPos < QUEUE_MAXCOUNT) && (aQueue->iPushPos < QUEUE_MAXCOUNT));

		if (aQueue->iPopPos == aQueue->iPushPos)
		{
			return 1;
		}
	}
	return 0;
}

char /* TQueueIterator */ queue_front(const struct TQueue *aQueue, TQueueElement *aValue)
{
	// Do paměti předané pomocí druhého parametru zapíše kopii elementu z čela fronty.
	// Pokud fronta existuje, není prázdná a druhý parametr není NULL,
	// zkopíruj hodnotu elementu z čela fronty do paměti předané pomocí ukazatele aValue a vrať 1,
	// jinak vrať 0.
	if (aQueue && !queue_is_empty(aQueue) && aValue)
	{
		*aValue = aQueue->iValues[aQueue->iPopPos];
		return 1;
	}

	return 0;
}

char /* TQueueIterator */ queue_back(const struct TQueue *aQueue, TQueueElement *aValue)
{
	// Do paměti předané pomocí druhého parametru zapíše kopii elementu z konce fronty.
	// Pokud fronta existuje, není prázdná a druhý parametr není NULL,
	// zkopíruj hodnotu elementu z konce fronty do paměti předané pomocí ukazatele aValue a vrať 1,
	// jinak vrať 0.
	// Využijte iPushPos, který již ovšem ukazuje na pozici pro uložení dalšího (budoucího) elementu.
	// Výpočet indexu elementu na konci fronty musí obsahovat operaci modulo.
	if (aQueue && !queue_is_empty(aQueue) && aValue)
	{
		*aValue = aQueue->iValues[(aQueue->iPushPos + QUEUE_MAXCOUNT - 1) % QUEUE_MAXCOUNT];
		return 1;
	}
	return 0;
}

char queue_push(struct TQueue *aQueue, TQueueElement aValue)
{
	// Vkládá element na konec fronty (dle indexu iPushPos).
	// Pokud parametr typu ukazatel na TQueue není NULL,
	// vypočti nový index pro příští uložení (iPushPos; použít modulo),
	// pokud je nový index pro uložení příštího elementu roven indexu elementu na čele fronty (iPopPos),
	// vrať 0.
	// Jinak na pozici (iPushPos) ulož vkládaný element aValue,
	// zapiš nový index pro uložení příštího elementu do iPushPos a vrať 1.
	if (aQueue)
	{
		size_t newpos = (aQueue->iPushPos + 1) % QUEUE_MAXCOUNT;
		if (newpos == aQueue->iPopPos)
		{
			return 0;
		}
		aQueue->iValues[aQueue->iPushPos] = aValue;
		aQueue->iPushPos = newpos;
	}
	return 1;
}

char queue_pop(struct TQueue *aQueue)
{
	// Odebere element z čela fronty (dle indexu iPopPos).
	// Pokud parametr typu ukazatel na TQueue není NULL a fronta není prázdná,
	// posuň čelo fronty na další element (iPopPos; použít modulo), vrať 1,
	// jinak vrať 0.
	if (aQueue && !queue_is_empty(aQueue))
	{
		aQueue->iPopPos = (aQueue->iPopPos + 1) % QUEUE_MAXCOUNT;
		return 1;
	}
	return 0;
}

void queue_destroy(struct TQueue *aQueue)
{
	// Korektně zruší všechny elementy fronty a uvede ji do základního stavu prázdné fronty (jako po queue_init).
	// je možné použít funkci queue_init (zdůvodněte).
	queue_init(aQueue);
}

struct TQueueIterator queue_iterator_begin(const struct TQueue *aQueue)
{
	// Inicializace a asociace/propojení iterátoru s frontou - zapíše odkaz na frontu a nastaví pozici v iterátoru na počátek fronty.
	// Pokud předaná fronta existuje (ukazatel není NULL) a není prázdná, ulož do iterátoru adresu asociované fronty,
	// nastav iterátor na element na čele fronty (na čele je element, který se bude první odebírat),
	// vrať hodnotu vytvořeného iterátoru.
	// Jinak vrať iterátor s vynulovanými vnitřními složkami.
	if (aQueue && !queue_is_empty(aQueue))
	{
		return (struct TQueueIterator){.iQueue = aQueue, .iPos = aQueue->iPopPos};
	}
	return (struct TQueueIterator){.iQueue = NULL, .iPos = 0};
}

char queue_iterator_is_valid(const struct TQueueIterator *aIter)
{ // Zjistí, zda iterátor odkazuje na platný element asociované fronty (tj. element má platnou pozici).
	// Pokud parametr typu ukazatel na TQueueIterator není NULL a
	// pokud je iterátor asociován s platnou frontou (tj. má platnou adresu TQueue) a tato fronta není prázdná, pokračuj.
	// Vrať 1, pokud je aktuální odkaz iterátoru platný (odkazuje do platných pozic mezi počátek a konec),
	// jinak vrať 0.
	if (aIter && aIter->iQueue && !queue_is_empty(aIter->iQueue) && (aIter->iPos != aIter->iQueue->iPushPos))
	{
		return 1;
	}
	return 0;
}

char queue_iterator_to_next(struct TQueueIterator *aIter)
{
	// Přesune odkaz v iterátoru z aktuálního elementu na následující element fronty.
	// Je-li iterátor validní pokračuj, jinak zruš propojení iterátoru s frontou a vrať 0.
	// Posuň aktuální odkaz na další element.
	// Vrať 1, když nově odkazovaný element existuje (odkazuje do platných pozic mezi počátek a konec; operace modulo),
	// jinak zruš propojení iterátoru s frontou a vrať 0.
	if (queue_iterator_is_valid(aIter))
	{
		aIter->iPos = (aIter->iPos + 1) % QUEUE_MAXCOUNT;
		if (aIter->iPos != aIter->iQueue->iPushPos)
		{
			return 1;
		}
		aIter->iQueue = NULL;
		aIter->iPos = 0;
	}
	return 0;
}

TQueueElement queue_iterator_value(const struct TQueueIterator *aIter)
{
	// Vrátí hodnotu elementu, na kterou odkazuje iterátor.
	// Pokud je iterátor validní, vrať hodnotu aktuálního elementu,
	// jinak vrať nulový element.
	if (queue_iterator_is_valid(aIter))
	{
		return (TQueueElement){aIter->iQueue->iValues[aIter->iPos]};
	}
	return (TQueueElement){0};
}

char queue_iterator_set_value(const struct TQueueIterator *aIter, TQueueElement aValue)
{
	// Nastaví element, na který odkazuje iterátor, na novou hodnotu.
	// Pokud je iterátor validní,
	// zapiš do aktuálního elementu hodnotu předanou pomocí druhého parametru a vrať 1,
	// jinak vrať 0.
	if (queue_iterator_is_valid(aIter))
	{
		((struct TQueue *)aIter->iQueue)->iValues[aIter->iPos] = aValue;
		return 1;
	}
	return 0;
}

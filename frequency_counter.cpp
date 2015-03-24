/* Frequency counter for Arduino 
   Should be accurate from 10Hz up to 5 or 8 Mhz
   Keeping in mind that ATmega328 has a general
   clock speed of 16Mhz 
   */

   /* Orinal can be found here http://www.gammon.com.au/timers */
   
   // Input: Pin D5

   // these are checked in the main program
   /* volatile keyword used to prevent compile optimizations
   	  that would produce undesired results for a value that 
   	  changes. Both values below could change from areas the
   	  compiler is not aware of.

   	  Check here for details 
   	  http://stackoverflow.com/questions/4437527/why-do-we-use-volatile-keyword-in-c
   	*/

   volatile unsigned long timerCounts;
   volatile boolean counterReady;

   // internal to counting routine
   unsigned long overflowCount;
   unsigned int timerTicks;
   unsigned int timerPeriod;

   void startCounting (usigned int ms) 
   {
   	counterReady = false;			// time not up yet
   	timerPeriod = ms;				// how many 1 ms counts to do
   	timerTicks = 0;					// reset interrupt counter
   	overflowCount = 0;				// no overflows yet

   	// reset Timer 1 and Timer 2
   	/* Using internal timers to follow time rather than 
   	   than timing programatically with millis() or some
   	   other technique 
	   ******************************************************
   	   More info go here --> https://arduinodiy.wordpress.com/2012/02/28/timer-interrupts/

   	   * This is also interesting
   	   "You can supply an external clock source for use with timers,
   	   but usually the chip’s internal clock is used as the clock source. 
   	   The 16 MHz crystal that is usually part of a setup for an Atmega328
   	   can be considered as part of the internal clock."

   	   Quoted from here --->
   	*/

   	TCCR1A = 0;
   	TCCR1B = 0;
   	TCCR2A = 0;
   	TCCR2B = 0;

   	// Timer 1 - counts events pin D5
   	TIMSK1 = bit (TOIE1); // interrupt on Timer 1 overflow

   	// Timer 2 - gives us our 1 mS counting interval
   	// 16 MHz clock (62.5 ns per tick) - prescaled by 128
   	// counter increments every 8 uS.
   	// So we count 125 of them, giving exactly 1000 uS ( 1 ms )

   	TCCR2A = bit (WGM21); // CTC mode
   	OCR2A  = 124;		  // count up to 125 (zero relative!!!)

   	// Timer 2 - interrup on match (ie. every 1 ms)
   	TIMSK2 = bit (OCIE2A);// enable Timer2 interrupt

   	TCNT1 = 0; 			  // both counters to zero
   	TCNT2 = 0;

   	// Reset prescalers
   	GTCCR = bit (PSRASY); // reset prescaler now
   	// start Timer 2
   	TCCR2B = bit (CS20) | bit (CS22); // prescaler of 128
   	// start Timer 1
   	// External clock source on T1 pin (D5). clock on rising edge.
   	TCCR1B = bit (CS10) | bit (CS11) | bit (CS12);

   }	// end startCounting

   ISR (TIMER1_OVF_vect)
   {
   		/* This is also why you have the 65535 multiplier.
   		   Because we are using a 16 bit register. */
   		++overflowCount;	// count number of Counter1 overflows
   }	// end of TIMER1_OVF_vect

   //*************************************************************
   //	Timer2 Interrupt Service is invoked by hardware Timer2 every 1ms = 1000 Hz
   // 16 Mhz / 128 / 125 = 1000 Hz

   ISR (TIMER2_COMPA_vect)
   {
		// grab counter value before it changes anymore
		usigned int timer1CounterValue;
		timer1CounterValue = TCNT1; // see datasheet, page 117 (accessing 16-bit registers)
		unsigned long overflowCopy = overflowCount;

		// see if we have reached timing period
		if (++timerTicks < timerPeriod)
			return; // not yet

		// if just missed an overflow
		if ((TIFR1 & bit (TOV1)) && timer1CounterValue < 256)
			overflowCopy++;

		// end of gate time, measurement ready

		TCCR1A = 0;			// stop timer 1
		TCCR1B = 0;

		TCCR2A = 0;			// stop timer2
		TCCR2B = 0;

		TIMSK1 = 0; 		// disable Timer1 Interrupt
		TIMSK2 = 0;			// disable Timer2 Interrupt

		// calculate total count
		timerCounts = (overflowCopy << 16) + timer1CounterValue; // each overflow is 65536 more
		counterReady = true;									 // set global flag for end of count period

   }	// end of TIMER2_COMPA_vect

   void setup ()
   {
   		Serial.begin(115200);
   		Serial.println("Frequency Counter"):

   }	// end of setup

   void loop()
   {
   		// stop Timer0 interrupts from throwing the count out
   		byte oldTCCR0A = TCCR0A;
   		byte oldTCCR0B = TCCR0B;

   		TCCR0A = 0;			// stop timer 0
   		TCCR0B = 0;

   		startCounting(500); // how many ms to count for

   		while (!counterReady)
   			{}	// loop until count over

   		// adjust counts by counting interval to give frequency in Hz
   		float frq = (timerCounts * 1000.0) / timerPeriod;

   		Serial.print("Frequency: ");
   		Serial.print((usigned long) frq); // casting here
   		Serial.println(" Hz.");

   		// restart timer 0
   		TCCR0A = oldTCCR0A;
   		TCCR0B = oldTCCR0B;

   		// let serial stuff finish
   		delay(200); // in milliseconds

   }	// end of loop
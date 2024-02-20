// lightweight random function from: https://en.wikipedia.org/wiki/Linear-feedback_shift_register
unsigned short lfsr = 0xACE1u;
unsigned bit;
unsigned long long r;

unsigned _rand()
{
    bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
    return lfsr =  (lfsr >> 1) | (bit << 15);
};

unsigned long long rand() {
    r |= (_rand() << 16);
    return r |= rand();
};

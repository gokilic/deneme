/* Based on the public domain implementation in
 * crypto_hash/keccakc512/simple/ from http://bench.cr.yp.to/supercop.html
 * by Ronny Van Keer
 * and the public domain "TweetFips202" implementation
 * from https://twitter.com/tweetfips202
 * by Gilles Van Assche, Daniel J. Bernstein, and Peter Schwabe */

#define MAKE_OLD_SHA3_LIKE_CORRECT_SHA3 0
#define USE_OLD_SHA3_FOR_KYBER 1

#if defined(WINDOWS)
#pragma warning(disable : 4244)
#endif

#include <assert.h>
#include <oqs/sha3.h>
#include <stdint.h>

#define SHAKE128_RATE OQS_SHA3_SHAKE128_RATE
#define SHA3_256_RATE OQS_SHA3_SHA3_256_RATE
#define SHA3_512_RATE OQS_SHA3_SHA3_512_RATE
#define NROUNDS 24
#define ROL(a, offset) ((a << offset) ^ (a >> (64 - offset)))

static uint64_t load64(const unsigned char *x) {
	unsigned long long r = 0, i;

	for (i = 0; i < 8; ++i) {
		r |= (unsigned long long) x[i] << 8 * i;
	}
	return r;
}

static void store64(uint8_t *x, uint64_t u) {
	unsigned int i;

	for (i = 0; i < 8; ++i) {
		x[i] = u;
		u >>= 8;
	}
}

static const uint64_t KeccakF_RoundConstants[NROUNDS] = {
    (uint64_t) 0x0000000000000001ULL,
    (uint64_t) 0x0000000000008082ULL,
    (uint64_t) 0x800000000000808aULL,
    (uint64_t) 0x8000000080008000ULL,
    (uint64_t) 0x000000000000808bULL,
    (uint64_t) 0x0000000080000001ULL,
    (uint64_t) 0x8000000080008081ULL,
    (uint64_t) 0x8000000000008009ULL,
    (uint64_t) 0x000000000000008aULL,
    (uint64_t) 0x0000000000000088ULL,
    (uint64_t) 0x0000000080008009ULL,
    (uint64_t) 0x000000008000000aULL,
    (uint64_t) 0x000000008000808bULL,
    (uint64_t) 0x800000000000008bULL,
    (uint64_t) 0x8000000000008089ULL,
    (uint64_t) 0x8000000000008003ULL,
    (uint64_t) 0x8000000000008002ULL,
    (uint64_t) 0x8000000000000080ULL,
    (uint64_t) 0x000000000000800aULL,
    (uint64_t) 0x800000008000000aULL,
    (uint64_t) 0x8000000080008081ULL,
    (uint64_t) 0x8000000000008080ULL,
    (uint64_t) 0x0000000080000001ULL,
    (uint64_t) 0x8000000080008008ULL};

static void KeccakF1600_StatePermute(uint64_t *state) {
	int round;

	uint64_t Aba, Abe, Abi, Abo, Abu;
	uint64_t Aga, Age, Agi, Ago, Agu;
	uint64_t Aka, Ake, Aki, Ako, Aku;
	uint64_t Ama, Ame, Ami, Amo, Amu;
	uint64_t Asa, Ase, Asi, Aso, Asu;
	uint64_t BCa, BCe, BCi, BCo, BCu;
	uint64_t Da, De, Di, Do, Du;
	uint64_t Eba, Ebe, Ebi, Ebo, Ebu;
	uint64_t Ega, Ege, Egi, Ego, Egu;
	uint64_t Eka, Eke, Eki, Eko, Eku;
	uint64_t Ema, Eme, Emi, Emo, Emu;
	uint64_t Esa, Ese, Esi, Eso, Esu;

	//copyFromState(A, state)
	Aba = state[0];
	Abe = state[1];
	Abi = state[2];
	Abo = state[3];
	Abu = state[4];
	Aga = state[5];
	Age = state[6];
	Agi = state[7];
	Ago = state[8];
	Agu = state[9];
	Aka = state[10];
	Ake = state[11];
	Aki = state[12];
	Ako = state[13];
	Aku = state[14];
	Ama = state[15];
	Ame = state[16];
	Ami = state[17];
	Amo = state[18];
	Amu = state[19];
	Asa = state[20];
	Ase = state[21];
	Asi = state[22];
	Aso = state[23];
	Asu = state[24];

	for (round = 0; round < NROUNDS; round += 2) {
		//    prepareTheta
		BCa = Aba ^ Aga ^ Aka ^ Ama ^ Asa;
		BCe = Abe ^ Age ^ Ake ^ Ame ^ Ase;
		BCi = Abi ^ Agi ^ Aki ^ Ami ^ Asi;
		BCo = Abo ^ Ago ^ Ako ^ Amo ^ Aso;
		BCu = Abu ^ Agu ^ Aku ^ Amu ^ Asu;

		//thetaRhoPiChiIotaPrepareTheta(round  , A, E)
		Da = BCu ^ ROL(BCe, 1);
		De = BCa ^ ROL(BCi, 1);
		Di = BCe ^ ROL(BCo, 1);
		Do = BCi ^ ROL(BCu, 1);
		Du = BCo ^ ROL(BCa, 1);

		Aba ^= Da;
		BCa = Aba;
		Age ^= De;
		BCe = ROL(Age, 44);
		Aki ^= Di;
		BCi = ROL(Aki, 43);
		Amo ^= Do;
		BCo = ROL(Amo, 21);
		Asu ^= Du;
		BCu = ROL(Asu, 14);
		Eba = BCa ^ ((~BCe) & BCi);
		Eba ^= (uint64_t) KeccakF_RoundConstants[round];
		Ebe = BCe ^ ((~BCi) & BCo);
		Ebi = BCi ^ ((~BCo) & BCu);
		Ebo = BCo ^ ((~BCu) & BCa);
		Ebu = BCu ^ ((~BCa) & BCe);

		Abo ^= Do;
		BCa = ROL(Abo, 28);
		Agu ^= Du;
		BCe = ROL(Agu, 20);
		Aka ^= Da;
		BCi = ROL(Aka, 3);
		Ame ^= De;
		BCo = ROL(Ame, 45);
		Asi ^= Di;
		BCu = ROL(Asi, 61);
		Ega = BCa ^ ((~BCe) & BCi);
		Ege = BCe ^ ((~BCi) & BCo);
		Egi = BCi ^ ((~BCo) & BCu);
		Ego = BCo ^ ((~BCu) & BCa);
		Egu = BCu ^ ((~BCa) & BCe);

		Abe ^= De;
		BCa = ROL(Abe, 1);
		Agi ^= Di;
		BCe = ROL(Agi, 6);
		Ako ^= Do;
		BCi = ROL(Ako, 25);
		Amu ^= Du;
		BCo = ROL(Amu, 8);
		Asa ^= Da;
		BCu = ROL(Asa, 18);
		Eka = BCa ^ ((~BCe) & BCi);
		Eke = BCe ^ ((~BCi) & BCo);
		Eki = BCi ^ ((~BCo) & BCu);
		Eko = BCo ^ ((~BCu) & BCa);
		Eku = BCu ^ ((~BCa) & BCe);

		Abu ^= Du;
		BCa = ROL(Abu, 27);
		Aga ^= Da;
		BCe = ROL(Aga, 36);
		Ake ^= De;
		BCi = ROL(Ake, 10);
		Ami ^= Di;
		BCo = ROL(Ami, 15);
		Aso ^= Do;
		BCu = ROL(Aso, 56);
		Ema = BCa ^ ((~BCe) & BCi);
		Eme = BCe ^ ((~BCi) & BCo);
		Emi = BCi ^ ((~BCo) & BCu);
		Emo = BCo ^ ((~BCu) & BCa);
		Emu = BCu ^ ((~BCa) & BCe);

		Abi ^= Di;
		BCa = ROL(Abi, 62);
		Ago ^= Do;
		BCe = ROL(Ago, 55);
		Aku ^= Du;
		BCi = ROL(Aku, 39);
		Ama ^= Da;
		BCo = ROL(Ama, 41);
		Ase ^= De;
		BCu = ROL(Ase, 2);
		Esa = BCa ^ ((~BCe) & BCi);
		Ese = BCe ^ ((~BCi) & BCo);
		Esi = BCi ^ ((~BCo) & BCu);
		Eso = BCo ^ ((~BCu) & BCa);
		Esu = BCu ^ ((~BCa) & BCe);

		//    prepareTheta
		BCa = Eba ^ Ega ^ Eka ^ Ema ^ Esa;
		BCe = Ebe ^ Ege ^ Eke ^ Eme ^ Ese;
		BCi = Ebi ^ Egi ^ Eki ^ Emi ^ Esi;
		BCo = Ebo ^ Ego ^ Eko ^ Emo ^ Eso;
		BCu = Ebu ^ Egu ^ Eku ^ Emu ^ Esu;

		//thetaRhoPiChiIotaPrepareTheta(round+1, E, A)
		Da = BCu ^ ROL(BCe, 1);
		De = BCa ^ ROL(BCi, 1);
		Di = BCe ^ ROL(BCo, 1);
		Do = BCi ^ ROL(BCu, 1);
		Du = BCo ^ ROL(BCa, 1);

		Eba ^= Da;
		BCa = Eba;
		Ege ^= De;
		BCe = ROL(Ege, 44);
		Eki ^= Di;
		BCi = ROL(Eki, 43);
		Emo ^= Do;
		BCo = ROL(Emo, 21);
		Esu ^= Du;
		BCu = ROL(Esu, 14);
		Aba = BCa ^ ((~BCe) & BCi);
		Aba ^= (uint64_t) KeccakF_RoundConstants[round + 1];
		Abe = BCe ^ ((~BCi) & BCo);
		Abi = BCi ^ ((~BCo) & BCu);
		Abo = BCo ^ ((~BCu) & BCa);
		Abu = BCu ^ ((~BCa) & BCe);

		Ebo ^= Do;
		BCa = ROL(Ebo, 28);
		Egu ^= Du;
		BCe = ROL(Egu, 20);
		Eka ^= Da;
		BCi = ROL(Eka, 3);
		Eme ^= De;
		BCo = ROL(Eme, 45);
		Esi ^= Di;
		BCu = ROL(Esi, 61);
		Aga = BCa ^ ((~BCe) & BCi);
		Age = BCe ^ ((~BCi) & BCo);
		Agi = BCi ^ ((~BCo) & BCu);
		Ago = BCo ^ ((~BCu) & BCa);
		Agu = BCu ^ ((~BCa) & BCe);

		Ebe ^= De;
		BCa = ROL(Ebe, 1);
		Egi ^= Di;
		BCe = ROL(Egi, 6);
		Eko ^= Do;
		BCi = ROL(Eko, 25);
		Emu ^= Du;
		BCo = ROL(Emu, 8);
		Esa ^= Da;
		BCu = ROL(Esa, 18);
		Aka = BCa ^ ((~BCe) & BCi);
		Ake = BCe ^ ((~BCi) & BCo);
		Aki = BCi ^ ((~BCo) & BCu);
		Ako = BCo ^ ((~BCu) & BCa);
		Aku = BCu ^ ((~BCa) & BCe);

		Ebu ^= Du;
		BCa = ROL(Ebu, 27);
		Ega ^= Da;
		BCe = ROL(Ega, 36);
		Eke ^= De;
		BCi = ROL(Eke, 10);
		Emi ^= Di;
		BCo = ROL(Emi, 15);
		Eso ^= Do;
		BCu = ROL(Eso, 56);
		Ama = BCa ^ ((~BCe) & BCi);
		Ame = BCe ^ ((~BCi) & BCo);
		Ami = BCi ^ ((~BCo) & BCu);
		Amo = BCo ^ ((~BCu) & BCa);
		Amu = BCu ^ ((~BCa) & BCe);

		Ebi ^= Di;
		BCa = ROL(Ebi, 62);
		Ego ^= Do;
		BCe = ROL(Ego, 55);
		Eku ^= Du;
		BCi = ROL(Eku, 39);
		Ema ^= Da;
		BCo = ROL(Ema, 41);
		Ese ^= De;
		BCu = ROL(Ese, 2);
		Asa = BCa ^ ((~BCe) & BCi);
		Ase = BCe ^ ((~BCi) & BCo);
		Asi = BCi ^ ((~BCo) & BCu);
		Aso = BCo ^ ((~BCu) & BCa);
		Asu = BCu ^ ((~BCa) & BCe);
	}

	//copyToState(state, A)
	state[0] = Aba;
	state[1] = Abe;
	state[2] = Abi;
	state[3] = Abo;
	state[4] = Abu;
	state[5] = Aga;
	state[6] = Age;
	state[7] = Agi;
	state[8] = Ago;
	state[9] = Agu;
	state[10] = Aka;
	state[11] = Ake;
	state[12] = Aki;
	state[13] = Ako;
	state[14] = Aku;
	state[15] = Ama;
	state[16] = Ame;
	state[17] = Ami;
	state[18] = Amo;
	state[19] = Amu;
	state[20] = Asa;
	state[21] = Ase;
	state[22] = Asi;
	state[23] = Aso;
	state[24] = Asu;

#undef round
}

#include <string.h>
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void keccak_absorb(uint64_t *s,
                          unsigned int r,
                          const unsigned char *m, unsigned long long int mlen,
                          unsigned char p) {
	unsigned long long i;
	unsigned char t[200];

#if !MAKE_OLD_SHA3_LIKE_CORRECT_SHA3
	// THIS SHOULD NOT BE HERE BUT IS PRESENT IN old_sha3
	// IF IT IS ABSENT, Kyber WORKS
	// IF IT IS PRESENT, Kyber DOESN'T WORK
	for (i = 0; i < 25; ++i)
		s[i] = 0;
#endif

	while (mlen >= r) {
		for (i = 0; i < r / 8; ++i)
			s[i] ^= load64(m + 8 * i);

		KeccakF1600_StatePermute(s);
		mlen -= r;
		m += r;
	}

	for (i = 0; i < r; ++i)
		t[i] = 0;
	for (i = 0; i < mlen; ++i)
		t[i] = m[i];
	t[i] = p;
	t[r - 1] |= 128;
	for (i = 0; i < r / 8; ++i)
		s[i] ^= load64(t + 8 * i);
}

void old_OQS_SHA3_keccak_squeezeblocks(unsigned char *h, unsigned long long int nblocks,
                                   uint64_t *s,
                                   unsigned int r) {
	unsigned int i;
	while (nblocks > 0) {
		KeccakF1600_StatePermute(s);
		for (i = 0; i < (r >> 3); i++) {
			store64(h + 8 * i, s[i]);
		}
		h += r;
		nblocks--;
	}
}

void old_OQS_SHA3_sha3256(unsigned char *output, const unsigned char *input, unsigned int inputByteLen) {
	uint64_t s[25];
	unsigned char t[SHA3_256_RATE];
	int i;

#if MAKE_OLD_SHA3_LIKE_CORRECT_SHA3
	// THIS SHOULD BE HERE BUT IS NOT PRESENT IN old_sha3
	for (i = 0; i < 25; ++i)
		s[i] = 0;
#endif

	keccak_absorb(s, SHA3_256_RATE, input, inputByteLen, 0x06);
	old_OQS_SHA3_keccak_squeezeblocks(t, 1, s, SHA3_256_RATE);
	for (i = 0; i < 32; i++)
		output[i] = t[i];
}

void old_OQS_SHA3_sha3512(unsigned char *output, const unsigned char *input, unsigned int inputByteLen) {
	uint64_t s[25];
	unsigned char t[SHA3_512_RATE];
	int i;
	//TODO: not sure about 0x80
	keccak_absorb(s, SHA3_512_RATE, input, inputByteLen, 0x80);
	old_OQS_SHA3_keccak_squeezeblocks(t, 1, s, SHA3_512_RATE);
	for (i = 0; i < 64; i++)
		output[i] = t[i];
}

void old_OQS_SHA3_shake128_absorb(uint64_t *s, const unsigned char *input, unsigned int inputByteLen) {
	keccak_absorb(s, SHAKE128_RATE, input, inputByteLen, 0x1F);
}

void old_OQS_SHA3_shake128_squeezeblocks(unsigned char *output, unsigned long long nblocks, uint64_t *s) {
	old_OQS_SHA3_keccak_squeezeblocks(output, nblocks, s, SHAKE128_RATE);
}

void old_OQS_SHA3_shake128(unsigned char *output, unsigned long long outlen,
                       const unsigned char *input, unsigned long long inlen) {
	uint64_t s[25];
	unsigned char t[SHAKE128_RATE];
	unsigned long long nblocks = outlen / SHAKE128_RATE;
	size_t i;

	for (i = 0; i < 25; ++i)
		s[i] = 0;

	/* Absorb input */
	keccak_absorb(s, SHAKE128_RATE, input, inlen, 0x1F);

	/* Squeeze output */
	old_OQS_SHA3_keccak_squeezeblocks(output, nblocks, s, SHAKE128_RATE);

	output += nblocks * SHAKE128_RATE;
	outlen -= nblocks * SHAKE128_RATE;

	if (outlen) {
		old_OQS_SHA3_keccak_squeezeblocks(t, 1, s, SHAKE128_RATE);
		for (i = 0; i < outlen; i++)
			output[i] = t[i];
	}
}

void old_OQS_SHA3_cshake128_simple_absorb(uint64_t s[25],
                                      uint16_t cstm, // 2-byte domain separator
                                      const unsigned char *in, unsigned long long inlen) {
	unsigned char *sep = (unsigned char *) s;

#if !MAKE_OLD_SHA3_LIKE_CORRECT_SHA3
	// THIS SHOULD NOT BE HERE BUT IS PRESENT IN old_sha3
	unsigned int i;
	for (i = 0; i < 25; ++i)
		s[i] = 0;
#endif

	/* Absorb customization (domain-separation) string */
	sep[0] = 0x01;
	sep[1] = 0xa8;
	sep[2] = 0x01;
	sep[3] = 0x00;
	sep[4] = 0x01;
	sep[5] = 16; // fixed bitlen of cstm
	sep[6] = cstm & 0xff;
	sep[7] = cstm >> 8;

	KeccakF1600_StatePermute(s);

	/* Absorb input */
	keccak_absorb(s, SHAKE128_RATE, in, inlen, 0x04);
}

void old_OQS_SHA3_cshake128_simple_squeezeblocks(unsigned char *output, unsigned long long nblocks, uint64_t *s) {
	old_OQS_SHA3_keccak_squeezeblocks(output, nblocks, s, SHAKE128_RATE);
}

void old_OQS_SHA3_cshake128_simple(unsigned char *output, unsigned long long outlen,
                               uint16_t cstm, // 2-byte domain separator
                               const unsigned char *in, unsigned long long inlen) {
	uint64_t s[25];
	unsigned char t[SHAKE128_RATE];
	unsigned int i;

#if MAKE_OLD_SHA3_LIKE_CORRECT_SHA3
	// THIS SHOULD BE HERE BUT IS NOT PRESENT IN old_sha3
	for (i = 0; i < 25; ++i)
		s[i] = 0;
#endif

	old_OQS_SHA3_cshake128_simple_absorb(s, cstm, in, inlen);

	/* Squeeze output */
	old_OQS_SHA3_keccak_squeezeblocks(output, outlen / SHAKE128_RATE, s, SHAKE128_RATE);
	output += (outlen / SHAKE128_RATE) * SHAKE128_RATE;

	if (outlen % SHAKE128_RATE) {
		old_OQS_SHA3_keccak_squeezeblocks(t, 1, s, SHAKE128_RATE);
		for (i = 0; i < outlen % SHAKE128_RATE; i++)
			output[i] = t[i];
	}
}

#include <stdio.h>
#define PRINT_HEX_STRING(label, str, len)                        \
	{                                                            \
		printf("%-20s (%4zu bytes):  ", (label), (size_t)(len)); \
		for (size_t i = 0; i < (len); i++) {                     \
			printf("%02X", ((unsigned char *) (str))[i]);        \
		}                                                        \
		printf("\n");                                            \
	}

void tester_OQS_SHA3_cshake128_simple(unsigned char *output, unsigned long long outlen, uint16_t cstm, const unsigned char *in, unsigned long long inlen) {
	unsigned char *out_new = malloc(outlen);
	unsigned char *out_old = malloc(outlen);
	OQS_SHA3_cshake128_simple(out_new, outlen, cstm, in, inlen);
	old_OQS_SHA3_cshake128_simple(out_old, outlen, cstm, in, inlen);
	if (0 != memcmp(out_old, out_new, outlen)) {
		fprintf(stderr, "Found mismatch in cshake128_simple\n");
#if 1
		PRINT_HEX_STRING("cstm", (unsigned char *) &cstm, 2)
		PRINT_HEX_STRING("in", in, inlen)
		PRINT_HEX_STRING("out_new", out_new, outlen)
		PRINT_HEX_STRING("out_old", out_old, outlen)
#endif
	}
	memcpy(output, USE_OLD_SHA3_FOR_KYBER ? out_old : out_new, outlen);
	free(out_new);
	free(out_old);
}

static void tester_OQS_SHA3_shake128(unsigned char *output, unsigned long long outlen, const unsigned char *input, unsigned long long inlen) {
	unsigned char *out_new = malloc(outlen);
	unsigned char *out_old = malloc(outlen);
	OQS_SHA3_shake128(out_new, outlen, input, inlen);
	old_OQS_SHA3_shake128(out_old, outlen, input, inlen);
	if (0 != memcmp(out_old, out_new, outlen)) {
		fprintf(stderr, "Found mismatch in shake128\n");
	}
	memcpy(output, USE_OLD_SHA3_FOR_KYBER ? out_old : out_new, outlen);
	free(out_new);
	free(out_old);
}

static void tester_OQS_SHA3_sha3256(unsigned char *output, const unsigned char *input, unsigned int inputByteLen) {
	unsigned char out_new[32];
	unsigned char out_old[32];
	OQS_SHA3_sha3256(out_new, input, inputByteLen);
	old_OQS_SHA3_sha3256(out_old, input, inputByteLen);
	if (0 != memcmp(out_old, out_new, 32)) {
		fprintf(stderr, "Found mismatch in sha3256\n");
#if 1
		PRINT_HEX_STRING("input", input, inputByteLen)
		PRINT_HEX_STRING("out_new", out_new, 32)
		PRINT_HEX_STRING("out_old", out_old, 32)
#endif
	}
	memcpy(output, USE_OLD_SHA3_FOR_KYBER ? out_old : out_new, 32);
}

static void tester_OQS_SHA3_cshake128_simple_absorb(uint64_t s[25], uint16_t cstm, const unsigned char *in, unsigned long long inlen) {
	uint64_t s_new[25];
	uint64_t s_old[25];
	memcpy(s_new, s, 25 * sizeof(uint64_t));
	memcpy(s_old, s, 25 * sizeof(uint64_t));
	OQS_SHA3_cshake128_simple_absorb(s_new, cstm, in, inlen);
	old_OQS_SHA3_cshake128_simple_absorb(s_old, cstm, in, inlen);
	if (0 != memcmp(s_old, s_new, 25 * sizeof(uint64_t))) {
		fprintf(stderr, "Found mismatch in cshake128_simple_absorb\n");
#if 1
		PRINT_HEX_STRING("s", s, 25 * sizeof(uint64_t))
		PRINT_HEX_STRING("cstm", (unsigned char *) &cstm, 2)
		PRINT_HEX_STRING("in", in, inlen)
		PRINT_HEX_STRING("s_new", s_new, 25 * sizeof(uint64_t))
		PRINT_HEX_STRING("s_old", s_old, 25 * sizeof(uint64_t))
#endif
	}
	memcpy(s, USE_OLD_SHA3_FOR_KYBER ? s_old : s_new, 25 * sizeof(uint64_t));
}

static void tester_OQS_SHA3_cshake128_simple_squeezeblocks(unsigned char *output, unsigned long long nblocks, uint64_t *s) {
	uint64_t s_new[25];
	uint64_t s_old[25];
	unsigned char *output_new = malloc(SHAKE128_RATE * nblocks);
	unsigned char *output_old = malloc(SHAKE128_RATE * nblocks);
	memcpy(s_new, s, 25 * sizeof(uint64_t));
	memcpy(s_old, s, 25 * sizeof(uint64_t));
	OQS_SHA3_cshake128_simple_squeezeblocks(output_new, nblocks, s_new);
	old_OQS_SHA3_cshake128_simple_squeezeblocks(output_old, nblocks, s_old);
	if (0 != memcmp(output_old, output_new, SHAKE128_RATE * nblocks)) {
		fprintf(stderr, "Found mismatch in cshake128_simple_squeezeblocks\n");
#if 0
		PRINT_HEX_STRING("s", s, 25 * sizeof(uint64_t))
		PRINT_HEX_STRING("s_new", s_new, 25 * sizeof(uint64_t))
		PRINT_HEX_STRING("s_old", s_old, 25 * sizeof(uint64_t))
		PRINT_HEX_STRING("output_new", output_new, SHAKE128_RATE * nblocks)
		PRINT_HEX_STRING("output_old", output_old, SHAKE128_RATE * nblocks)
#endif
	}
	memcpy(s, USE_OLD_SHA3_FOR_KYBER ? s_old : s_new, 25 * sizeof(uint64_t));
	memcpy(output, USE_OLD_SHA3_FOR_KYBER ? output_old : output_new, SHAKE128_RATE * nblocks);
	free(output_new);
	free(output_old);
}

#ifndef RUNLENGTH_H
#define RUNLENGTH_H

#include "Constraint.h"
#include "Automaton.h"

//
// RunLength: 
//
template <size_t M>
class RunLength : public Constraint<bool>
{
    public:
        RunLength(vector<int> & length);
        bool OnDecided(Variable<bool> *decided);
        bool Enforce();

    private:
        vector<int> & length;
        Automaton<bool,2,M>  automaton;

        class Line : public Automaton<bool,2,M>::template Input<Line> {
            public:
                bool operator ==(Line &line);
                uint64_t Hash() const;
                void Show() const;

                bool IsDecided(size_t i) const  { return decided.Has(i); }
                void SetDecided(size_t i)       { decided.Add(i); }
                bool GetValue(size_t i) const   { return value.Has(i); }
                void SetValue(size_t i, bool v) { if (v) value.Add(i); else value.Remove(i); }

            private:
                Set<M> value;
                Set<M> decided;
        };

        class Cache {
            public:
                bool Lookup(Line &in, Line &out);
                void Update(Line &in, Line &out);

            private:
                static const size_t BUCKETS = 1024;
                Line input[BUCKETS];
                Line output[BUCKETS];
        };

        Cache cache;

        bool Accept(Line &out);
};

template <size_t M>
RunLength<M>::RunLength(vector<int> & length)
    : length(length)
{
    typedef typename Automaton<bool,2,M>::Run   Run;
    vector<Run> run;

    for (size_t i = 0; i < length.size(); i++) {
        run.push_back(Run(false, i != 0, Run::AT_LEAST));
        run.push_back(Run(true, length[i], Run::EQUAL));
    }
    run.push_back(Run(false, 0, Run::AT_LEAST));

    new (&automaton) Automaton<bool,2,M>(run);
}

template <size_t M>
bool RunLength<M>::OnDecided(Variable<bool> *decided)
{
    return Enforce();
}

template <size_t M>
bool RunLength<M>::Enforce()
{
    vector<Variable<bool> *>  &variables = Constraint<bool>::variables;
    size_t num_variables = variables.size();

    Line out;
    if (!Accept(out))
        return false;

    for (size_t i = 0; i < num_variables; i++) {
        if (variables[i]->GetDomainSize() > 1 && out.IsDecided(i)) {
            variables[i]->Decide(out.GetValue(i));

            vector<Constraint<bool>*> &constraints = variables[i]->GetConstraints();
            for (size_t j = 0; j < constraints.size(); j++) {
                if (constraints[j] != this)
                    problem->ActivateConstraint(constraints[j]);
            }
        }
    }

    return true;
}

template <size_t M>
bool RunLength<M>::Accept(Line &out)
{
    vector<Variable<bool> *>  &variables = Constraint<bool>::variables;
    size_t num_inputs = variables.size();

    Line in;
    for (size_t i = 0; i < num_inputs; i++) {
        if (variables[i]->GetDomainSize() == 1) {
            in.SetValue(i, variables[i]->GetValue(0));
            in.SetDecided(i);
        } else {
            in.SetValue(i, true);
        }
    }

    if (cache.Lookup(in, out))
        return out.IsDecided(M); // accept flag is the last bit

    out = in;
    if (automaton.Accept(out, num_inputs)) {
        out.SetDecided(M); // accept flag is the last bit
        cache.Update(in, out);
        // printf("%p %zu accepts:", this, in.Hash() % 1024); in.Show(); putchar('\n');
        return true;
    } else {
        cache.Update(in, out);
        // printf("%p %zu rejects:", this, in.Hash() % 1024); in.Show(); putchar('\n');
        return false;
    }
}

template <size_t M>
bool RunLength<M>::Line::operator ==(Line &line)
{
    return value == line.value && decided == line.decided;
}
                
template <size_t M>
uint64_t RunLength<M>::Line::Hash() const
{ 
    static const uint64_t hash_rand1[2][128] = {
        { 0x71e2db7423f5cc0fULL, 0x0e0711f851987110ULL, 0x7b2e48012c89e75dULL, 0x663ad94b609d166eULL, 0x564cef1a58bcaf37ULL, 0x4cf7c8eb13816a03ULL, 0x7360047c37eed96cULL, 0x7ea7d9644014f47aULL, 0x1f0656924cdf5ca3ULL, 0x7c3403da72f0b8f8ULL, 0x081caeda606d6c0fULL, 0x59dbab2160a12d7cULL, 0x7a2f86ec1a26177bULL, 0x1878a68a651896f3ULL, 0x292838e85b70fb0cULL, 0x5064c6481b0b145dULL, 0x7f66c71b5e6bd840ULL, 0x6ca3856d7a950f1dULL, 0x0af5bf9e52de5eb8ULL, 0x5b32258b6142aeb8ULL, 0x2b9b0df02829ee77ULL, 0x74c418bb1efb126cULL, 0x6018c7e3736bf21fULL, 0x5f1006e67f1f1e76ULL, 0x404b4ec25b440ac0ULL, 0x720fd76e4867fd9cULL, 0x3bb176d04beb828fULL, 0x29092b1835e0fdbcULL, 0x66119a0a4181d1a3ULL, 0x1af994b00f39d2f2ULL, 0x1cf2ccaf6b5e5af8ULL, 0x2a44e74f1c5993caULL, 0x49ca333816e86cbcULL, 0x16eea2e754bff2d6ULL, 0x69c6cb757220c873ULL, 0x3602a18e1561d965ULL, 0x1a4ab6ea2ac6ba49ULL, 0x345cebd17a637ecdULL, 0x1e32ac69136cf2b7ULL, 0x79829d435e7dfb2bULL, 0x6eb0fd786b9274b1ULL, 0x26e5f8c82a627448ULL, 0x377df7404fef23e0ULL, 0x604372041d8f914aULL, 0x1170f5837b3d06b4ULL, 0x2cc9643d2e63c232ULL, 0x669b61ac570e4b8cULL, 0x4abd55fd306594e5ULL, 0x6df6b84961abf8e4ULL, 0x052587bb57bd83beULL, 0x53ccc1573b28294aULL, 0x6d1f5d236e177841ULL, 0x65eee393217c48f4ULL, 0x687af70f04218ffcULL, 0x34e93bab61fd9452ULL, 0x629f8b28239a3923ULL, 0x4d900904098583f0ULL, 0x4dfcad6b050e0044ULL, 0x5974a7d02e401f70ULL, 0x229d918f6ae59d54ULL, 0x297d26244f66f5ccULL, 0x19495f86101887d1ULL, 0x267541586406b583ULL, 0x407e1cb6146bf9a1ULL, 0x45b2ae6845a3a471ULL, 0x6c297d5f197f6fbfULL, 0x00cbcdbb5948da82ULL, 0x0796e80166bab14fULL, 0x7ac523767011df10ULL, 0x6adc414b2fae5f22ULL, 0x520f73624d7bcc73ULL, 0x534898451f9f7c66ULL, 0x57015063214545b1ULL, 0x24ad7cab3075f834ULL, 0x4f856521474b0e3aULL, 0x1b5b958879028b45ULL, 0x16b2040634a4f50eULL, 0x091b13163d27455eULL, 0x18abaa9249992fccULL, 0x51933f005e5e58faULL, 0x0f3cd43e3dbcbc5fULL, 0x77ddc8b91008a1f9ULL, 0x170596e27f74b0baULL, 0x76c3534811caba58ULL, 0x6f868fca619f9494ULL, 0x4179197a4196032dULL, 0x2f1b610714c1b1c0ULL, 0x61357f93061cb16bULL, 0x3606f77105e2fc3eULL, 0x3692a99f058c5c92ULL, 0x4d2e0a7851ee3f27ULL, 0x7e8ee7d763e00e7eULL, 0x0693343507a9faeeULL, 0x210753dd1f3edec7ULL, 0x51432aba729a92ddULL, 0x7d9d37c1607ffef8ULL, 0x30574f3c757b007bULL, 0x7088a0f2475ce61eULL, 0x74efb135674bf43aULL, 0x5927a07764764100ULL, 0x48eb88ce1aa0b9f1ULL, 0x260c442d7806e9d6ULL, 0x2f626bb10741c3c0ULL, 0x7e239b4165696322ULL, 0x0d24bfff34b644e0ULL, 0x6af5bfb45a52ca77ULL, 0x06a484076984a78cULL, 0x3e32d8f60d37b83cULL, 0x712ea27a5f3a2cd3ULL, 0x2c7697044271cd34ULL, 0x51d4bfb02a13cec5ULL, 0x22f1cc2d022c0eecULL, 0x1f8ecf40137a6d1fULL, 0x4988f50b147e8076ULL, 0x7ac6615922b09582ULL, 0x78f4c17643b1ea28ULL, 0x3d514f731f0105a3ULL, 0x3bb8d3fe6cb3bb25ULL, 0x2642c96339dc6f3fULL, 0x521d1e4733678962ULL, 0x6e92b41f3d12ddfcULL, 0x0dba53da75373826ULL, 0x269785884bed2cd0ULL, 0x026ef06217c62802ULL, 0x2b2759a32ee58766ULL, 0x5a37f5367cfc1953ULL, 0x58f9562c7d29c163ULL, 0x7f28283f7888256cULL }, 
        { 0x10a42e8248b11d4aULL, 0x0d06a5e20b6a8fdcULL, 0x6b61b2cc05fb6758ULL, 0x4f1c7a0428b30240ULL, 0x24fc6cfb0ad54e02ULL, 0x1566bd654b3f365fULL, 0x44b1bd416783dbacULL, 0x7ea6bfc133447160ULL, 0x2496b9a80c61139bULL, 0x287ba9864b2e3f30ULL, 0x584e406b2aea99e8ULL, 0x62f4673203759a0eULL, 0x59d0214f3d2c5c69ULL, 0x0071b36132c9777bULL, 0x3a561dcc7f99dba1ULL, 0x2b519ce74afa4c4fULL, 0x484af8eb385842caULL, 0x5664dc2b33acabb8ULL, 0x3e53aa222581562fULL, 0x5c5fadf86350171eULL, 0x3056a43171c66b5dULL, 0x2e8f4d7d75086172ULL, 0x594a47092d360d3eULL, 0x284cd2d27de100b2ULL, 0x399720da50c87c58ULL, 0x490f3fe211e56145ULL, 0x7bb316402c03a715ULL, 0x155afb545583378fULL, 0x6930037e15ccaeb5ULL, 0x084caf0a2386214aULL, 0x15668a56339e4bf2ULL, 0x6e806d995db18342ULL, 0x6bf68ebc44e549c4ULL, 0x115e2efa2a4a38deULL, 0x6a669ff36dbddcf2ULL, 0x0d9a4ffc1abd4424ULL, 0x5f84484f3c299d79ULL, 0x0fc5a59638ce8f58ULL, 0x695faab838127868ULL, 0x36af900a22f6cb92ULL, 0x08daf4c07fbecfedULL, 0x34dc2cd7048e0b01ULL, 0x2bc277024a37282bULL, 0x5a11429014f27a80ULL, 0x6003d6e1625df19bULL, 0x38789bca756a6137ULL, 0x15fc3d8d26f90964ULL, 0x531be47901f2cc49ULL, 0x6bde5328647a1373ULL, 0x2c3d05275644f31cULL, 0x5237f06539d75524ULL, 0x7102374031bc38b4ULL, 0x7600f29d00c7dcd7ULL, 0x6a8ac80d5f609d55ULL, 0x38da553f213a5817ULL, 0x025768e741b54a00ULL, 0x20f92804373395bfULL, 0x464355014cbb9f06ULL, 0x016abdea20549791ULL, 0x61ae1986616e94cbULL, 0x02b2892c1a26b551ULL, 0x56d8f60318aec6b9ULL, 0x411fbeb529f4da7cULL, 0x1aa193022cfe11ddULL, 0x0e6eedf046de982aULL, 0x034304f960a6de55ULL, 0x00b5ed4e74453c3aULL, 0x1263170a76b6dfebULL, 0x750d19117ceddf17ULL, 0x56177d412de76e50ULL, 0x1e28372e586ee628ULL, 0x6f9cb8503f215f33ULL, 0x0fa27be735e00d51ULL, 0x0bdcfe39110d39d2ULL, 0x5634a4e36d8b17c0ULL, 0x727bce9d58e72e0fULL, 0x07b1cd114954c4a0ULL, 0x7195f4c948d18bc6ULL, 0x73499f1d0c3787cbULL, 0x75cf9da301b88d0dULL, 0x53161ff57912a29dULL, 0x625f6b6253cc0d43ULL, 0x6d57ded774c2826cULL, 0x4a82ed2f6264f7e8ULL, 0x71b06183209a6a70ULL, 0x104c66380fd898b2ULL, 0x790950987fe91e89ULL, 0x4ef9f7e508abcc80ULL, 0x35c92bda5ad6f61eULL, 0x19b906520bfdd0bdULL, 0x48620dde0c34d4efULL, 0x64e4fecd5013daefULL, 0x55899990567af396ULL, 0x18e566b548d338adULL, 0x62b27b610eb50459ULL, 0x4a8bc5ba35c89b57ULL, 0x07c7a6f62ceb311cULL, 0x0994a89a751f85cdULL, 0x21adb389541795c9ULL, 0x57847db5135e150cULL, 0x74b2003967d0e3edULL, 0x2336adbe6dbb50d2ULL, 0x67ba02767230a5a3ULL, 0x76671d521d832e51ULL, 0x4d079bc2102023a4ULL, 0x2980ff0e1569a9a0ULL, 0x1c54f8930e65fddbULL, 0x657d849071de9223ULL, 0x64e0f1717e62eb45ULL, 0x3ab1cad047936cd3ULL, 0x0d17ef9e053d908aULL, 0x7d5c082a14df9694ULL, 0x3228c1a706f0b0c4ULL, 0x09ff1c6153d67530ULL, 0x5b08468e61839a16ULL, 0x67348a3c4fba46c7ULL, 0x49547e040a6b37fbULL, 0x3d759799310e807aULL, 0x7c9bdd9e33dcb4ebULL, 0x4e91aecb49a37960ULL, 0x43fcd88f7812addaULL, 0x5f0d23016051d123ULL, 0x0678abb5448aa791ULL, 0x523063466b599d27ULL, 0x42ed92d60ce22e17ULL, 0x32ed09fa50058275ULL, 0x121fbea130491224ULL, 0x64e5190944488048ULL }
    }; 

    static const uint64_t hash_rand2[2][128] = {
        { 0x3739c2e86ee4356bULL, 0x181ef57812420976ULL, 0x5067cf817f537fb5ULL, 0x61fc503e19bc4d85ULL, 0x09beb7b01f71e7d7ULL, 0x4acace00065a954eULL, 0x534e9cc3195c7ccbULL, 0x4ffe0eaf174b7552ULL, 0x116f2aa52f0b31b0ULL, 0x779d467517e7d65bULL, 0x7395d94149cda9bcULL, 0x0341738236836c17ULL, 0x56afd7d3362e7d7cULL, 0x0688ee8c68cf9674ULL, 0x66778fa06b6e0796ULL, 0x2d1816bd1db15288ULL, 0x5a523d0145370c35ULL, 0x2ff35bff2aba0c82ULL, 0x448a8bea11efac3dULL, 0x44765a084e49439aULL, 0x316194140f412808ULL, 0x54a3d8e904b030d7ULL, 0x289da4d324a1e798ULL, 0x1bfba62a3a0ccf79ULL, 0x53ad19481398ec9fULL, 0x51f4a5d44742f289ULL, 0x5d66965b55361956ULL, 0x7dc65ea034166e2eULL, 0x0b6496d2044f4d2dULL, 0x1ce604a371dc2672ULL, 0x6fbd54c349fe1b60ULL, 0x0f8d78fa4a0f91c4ULL, 0x0f3527953f80d4f9ULL, 0x74c99e4653bfb380ULL, 0x51708136393ff84eULL, 0x2208f71a02d2154bULL, 0x4881205676acd003ULL, 0x07824622711ec52aULL, 0x1b4eb79b237dec4cULL, 0x2b2b94a36efbd0e3ULL, 0x3716d8ec7d203a77ULL, 0x363ec36c147d6f47ULL, 0x525653cd3405220dULL, 0x4893dd765dbaea9fULL, 0x38546f3a6579e219ULL, 0x4f9711112811c3fdULL, 0x2f77fd795f248a0bULL, 0x722155c13ead250eULL, 0x1ea55f0566eaf407ULL, 0x126cd88e7015e03bULL, 0x202aec563475cfa9ULL, 0x72e7f58668ac0cacULL, 0x2b229fac7a6a3ba9ULL, 0x59cad1d646715748ULL, 0x1de827f504f66679ULL, 0x356d282b54ff00e1ULL, 0x0216a0f06babeb98ULL, 0x697c7029546cf4bdULL, 0x1fb10da532104d9fULL, 0x3227df5c58057cdfULL, 0x178a2fb801bef06dULL, 0x001740dc47022d31ULL, 0x60e37a797238969dULL, 0x05af523f7f88d97eULL, 0x59238aa4181c2aceULL, 0x6f9eb9b9794e76faULL, 0x4c91fa776286af40ULL, 0x61fa83a777b49a23ULL, 0x5cf0eae93bc5557dULL, 0x3e25f16b7ad912deULL, 0x40bbbbf773931997ULL, 0x4fd813c042d25ce7ULL, 0x5f3f052f395483e9ULL, 0x173f51a57ef012d4ULL, 0x6b64d18849673101ULL, 0x56f58fb302ef0140ULL, 0x4b26216f570cd08fULL, 0x49f12e712c099be8ULL, 0x4945672c4fa080b0ULL, 0x2b9275662268f1d0ULL, 0x67bcab7e1b312f1fULL, 0x1bb768cb344ea5f5ULL, 0x7db7de5f7db1ec72ULL, 0x2c0340195aa8c948ULL, 0x397741ef6a293184ULL, 0x5581dc277a32fde6ULL, 0x5dbc4b1b2559efe7ULL, 0x3d055ace3cfb504aULL, 0x5eae73d05444ac73ULL, 0x3beb631e4a134558ULL, 0x1dabdd7412e0f2d1ULL, 0x4d02469868d1fee3ULL, 0x69edc36016f37509ULL, 0x14db9acb33332a8cULL, 0x6693f5b9406e1031ULL, 0x559c1c5d4e50a138ULL, 0x5b9f3f5171538528ULL, 0x029f472d59571db0ULL, 0x6f05719a2ea28746ULL, 0x33ffe6f9287cb389ULL, 0x18cbb8cb0981c320ULL, 0x22afb170768803e6ULL, 0x2edbb3075fb50c3eULL, 0x338354310d8a26d7ULL, 0x33f9b8b16f6eb74fULL, 0x579d6c2f51a59625ULL, 0x024faa21249fb2c7ULL, 0x3a7795096c3d6d81ULL, 0x3b9327d04f532fd4ULL, 0x1f70980e22271d89ULL, 0x0fc14006750cb46bULL, 0x7077bec16b607f57ULL, 0x66603993731705efULL, 0x44b79d075565ab2dULL, 0x21b98d3578b78400ULL, 0x7de25eb63a854600ULL, 0x0239472020921026ULL, 0x310d49e73114fa27ULL, 0x00471c6464909e18ULL, 0x3e9f20fe3440d515ULL, 0x53ff5567163c8d2dULL, 0x05e66b3b564eff88ULL, 0x3adc3ff4405e0044ULL, 0x428c6d0a766f67c4ULL, 0x0fb1301861fd0518ULL, 0x1896854e1f72701eULL, 0x5709b983090e440fULL, 0x0ad2ef753d69f316ULL } , 
        { 0x7c2549fe4f8a8c7dULL, 0x12cf9e431dded734ULL, 0x4842107d10b1fcf9ULL, 0x58641d344a7b579eULL, 0x31440d200971671bULL, 0x7b9051c5318b2984ULL, 0x6e0205333a2f72c4ULL, 0x65cbfe9a42015a9bULL, 0x506bfff16bb269d5ULL, 0x18505a230b483fe6ULL, 0x2c106a195adcc72dULL, 0x01b7a7aa3bc19a31ULL, 0x3cd9cc451a4e2cf8ULL, 0x5b340a5013e385c8ULL, 0x235c71086606f9c5ULL, 0x514d78de1f81bb06ULL, 0x35918642641d1721ULL, 0x3d60923a7dd396c0ULL, 0x74cf141b15c4af6fULL, 0x484eee5e2613213bULL, 0x1f36168a43df4023ULL, 0x579e4abf0d381bbeULL, 0x7e0eb2e73d6a4959ULL, 0x4f3976594e7ab2d9ULL, 0x291cb32e6789d07cULL, 0x59c2f2bf552d1d47ULL, 0x426697aa5b7a9a69ULL, 0x10eeb7797f4063efULL, 0x75c8c7626c22c1c9ULL, 0x1323e9b81925386aULL, 0x5229bb8e64716296ULL, 0x38a6f37007bb41d1ULL, 0x488e79b8760785abULL, 0x058ed8913d5d8dd3ULL, 0x0bcc351a4dddc6efULL, 0x6370af0e2b024ba4ULL, 0x11bd07123b0ef9cdULL, 0x383a67620fcbb9faULL, 0x787943270773ddbbULL, 0x5e466cd32195f655ULL, 0x6efdae3838095f92ULL, 0x76c3139d316445e2ULL, 0x1383f9fb07b1cb16ULL, 0x30a4a9d1094cc15dULL, 0x73d48cdf43c89389ULL, 0x2271f9c745fe486dULL, 0x2839f6205b18ed38ULL, 0x4db98a3e70c86fd8ULL, 0x512072e3534862cfULL, 0x2e25fdab5ceca7fdULL, 0x212629be1196acb9ULL, 0x07eef3a132e330d1ULL, 0x4ca5a68640295b04ULL, 0x42aeeacb451ee9adULL, 0x479d38bf20f5579eULL, 0x66b4e003369ae6f7ULL, 0x58feb7305d77f3a0ULL, 0x67ff2cd96c82b12bULL, 0x6529beb618a3d6abULL, 0x75cf728958fe4b95ULL, 0x5c6c6a3418416c50ULL, 0x1efc940204a66054ULL, 0x735a59886cb61e41ULL, 0x756ed02c447acc6bULL, 0x3ffe81102394cdd7ULL, 0x216774686124aacfULL, 0x352b7a902956680aULL, 0x1407dba001d12117ULL, 0x697fc30e56b6c66bULL, 0x46f00ac4311cfbcdULL, 0x77ac1e092da4eac7ULL, 0x67b7e2c550aad539ULL, 0x0b1cde674fb70f9eULL, 0x3d2d866470469d1dULL, 0x685ae64932fcf8edULL, 0x4944e8b244c7507eULL, 0x4b3e653e68417cb5ULL, 0x496db0d23e98bec6ULL, 0x54f79af63edc80ffULL, 0x03138b3214f61c06ULL, 0x62714ed6247aff9aULL, 0x761ac6d5179cc967ULL, 0x4dd167a40a22a275ULL, 0x196dea7e37512ab2ULL, 0x60d968e0605df542ULL, 0x686e2680588586e9ULL, 0x0e02e00a50260945ULL, 0x29305c22191fbe71ULL, 0x1fdd18e3665de287ULL, 0x09665b8f0837ff2dULL, 0x195adb7452ab4441ULL, 0x4cff4fab649940b2ULL, 0x3aecc0f6166d007dULL, 0x2331ff790fe45becULL, 0x5549817c26458aabULL, 0x24da77f337bad053ULL, 0x4ac08a451af53ec8ULL, 0x4f5799ba1891f1eaULL, 0x2517e13e68c58438ULL, 0x4fe31c9c05f14a1eULL, 0x4923797a3851431cULL, 0x5e76d10857265984ULL, 0x08774c6107a72d2aULL, 0x704617f628546545ULL, 0x6e050fb179ac7385ULL, 0x308c6472075feb26ULL, 0x4c57b7c67d8bb41dULL, 0x6bf92bd8074478bdULL, 0x13f8b49a0f2b2b51ULL, 0x1728d4a969423617ULL, 0x3570b5fc3c034c9cULL, 0x20fd066a00314042ULL, 0x56f88b657054a024ULL, 0x18c3322c7c106ca3ULL, 0x591a245c68a64ec8ULL, 0x0201b6c1223d9dd6ULL, 0x20f791e5607887c9ULL, 0x7963f75b296ede46ULL, 0x681fb4f469aa0f51ULL, 0x51c3438b5624c4a5ULL, 0x635682d6024fa7fdULL, 0x5d84afcb2fae3a9cULL, 0x7fdb5c1a497ddba4ULL, 0x36f2b35913d410b5ULL, 0x58a906f54e1b8803ULL, 0x7d1646cc0e19bcf2ULL, 0x0a1ed49f1e134d36ULL, 0x0e4afd3461176004ULL }
    };

    uint64_t sum = 0;
    for (size_t i = 0; i < M; i++)
        sum += hash_rand1[value.Has(i)][i] + hash_rand2[decided.Has(i)][i];
    return sum;
}

template <size_t M>
void RunLength<M>::Line::Show() const
{
    for (size_t i = 0; i < M; i++)
        putchar(IsDecided(i) ? GetValue(i) + '0' : '?');
}

template <size_t M>
bool RunLength<M>::Cache::Lookup(Line &in, Line &out)
{
    uint64_t index = in.Hash() % BUCKETS;
    if (in == input[index]) {
        out = output[index];
        return true;
    } else
        return false;
}

template <size_t M>
void RunLength<M>::Cache::Update(Line &in, Line &out)
{
    uint64_t index = in.Hash() % BUCKETS;
    input[index]  = in;
    output[index] = out;
}

#endif


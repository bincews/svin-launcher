// SPDX-License-Identifier: GPL-3.0-only

#pragma once

struct ElyAuthlib {
    const char* originalVersion;
    const char* patchedVersion;
    const char* sha256;
};

// Match the exact Authlib ABI supplied by Minecraft. These are PineconeMC's Ely.by builds.
constexpr ElyAuthlib elyAuthlibs[] = {
    { "1.2", "1.2-ely.1", "afba10464517c7092cd78cc464ad71119bb79eadf4cc815478ad41b4d631c155" },
    { "1.3", "1.3-ely.1", "193ad65c432f0d0e17ff2b4f5b5cd1f85116dd901cf0bb6c6eb01fdbf159f140" },
    { "1.3.1", "1.3.1-ely.1", "6386e1b26a90bb3fdae54b30b76bf3b32bfba481f32369277e12eef64ffc081c" },
    { "1.5.6", "1.5.6-ely.5", "871f4bb5a6a4308c3cd2f544e3b836be498a920946612344c5872445ee9a679a" },
    { "1.5.8", "1.5.8-ely.5", "df6fe0742aa866ac216b43f49d6cbf34370329a6093470b25f0091bd1d37e642" },
    { "1.5.12", "1.5.12-ely.5", "c418f2bfac8fcfcaac80a781c51580a3bab5628ed4c3b7170f7ba9a37774937f" },
    { "1.5.13", "1.5.13-ely.5", "8a556a34ac55cf07d7e044d02a47635812e6dd5e983a2094a21ecc30dfd65b0e" },
    { "1.5.21", "1.5.21-ely.4", "248a12966c81daaa286ad593111b1671902d628f9de1d0ed4000e0f02a947bfb" },
    { "1.5.22", "1.5.22-ely.4", "75f3c14abb3fb0733b6c77df24a9dffecba2823826595175d59555bea1e874c6" },
    { "1.5.24", "1.5.24-ely.4", "b46c39f2f88d5c3b494f13dc49a2a2cff121d963d36d2abe349b202b1c55d759" },
    { "1.5.25", "1.5.25-ely.4", "7432a70719cb5fee0e78b51f5523bc0bd230affae98bc197e29116fdf7b425f4" },
    { "1.6.25", "1.6.25-ely.4", "226c210117a8e2ef91cad7751dbdc965cdd1bac84f8c6dcaf9077701dda157c4" },
    { "2.1.28", "2.1.28-ely.5", "6b9b23c4f9bf8983ccdc27a79052ef70fc72426ff37be5bfa4d82e03094c55a8" },
    { "2.3.31", "2.3.31-ely.5", "c3ff8b39d0b1b6a4fa139bc903d2488c8070958272599cb24aa67884168942d0" },
    { "3.2.38", "3.2.38-ely.4", "44ca265507c5c732c8ceb895076498c0f9595585baf85ba2107ddc995ed47617" },
    { "3.3.39", "3.3.39-ely.5", "229d0f7363422bb5f0e8f7152617391d400138bbd3557e31bcd21af13c8721df" },
    { "3.5.41", "3.5.41-ely.6", "6472ceedd26cc8a65ea2ef14dfeeb71a1eaab8a31e61b6e48c5afb5f2053d0cd" },
    { "3.11.49", "3.11.49-ely.6", "3b9b36ba62a338ff3bcf0473ea68c081f01800d6a6f34ac834e863eeb0ac18f6" },
    { "3.11.50", "3.11.50-ely.6", "425209d6c1e92547aa7f7eb222f97e831157280113951c905dc094d5ee4a5f25" },
    { "3.16.29", "3.16.29-ely.6", "534b504b08d682d9c2b62a06b0c03c99f8b998ce3b27a7873b23155745102dcd" },
    { "3.18.38", "3.18.38-ely.6", "4273c22b3e6e42fbbd7187fd929e91f993200a220ad813e3602028c0c63789b7" },
    { "4.0.43", "4.0.43-ely.5", "36206ab209335d6a3aa7ddb9368581e55ea365ffb4a84f9f510e293c34c40c90" },
    { "5.0.46", "5.0.46-ely.5", "03b0cfc59fe5b12a8edd06f3a171682e806e83ba3f9dfb9eca570a6c02e0bc3a" },
    { "5.0.47", "5.0.47-ely.5", "477f9251a38bfa978a0eb82fc4473e886b03d53891453619e1630e1e7fb15078" },
    { "5.0.50", "5.0.50-ely.5", "7a17491d2f50d9ecfae3ffb3056f3f0bb8ac1e01e53ac9d45c03e37972b0fb4f" },
    { "5.0.51", "5.0.51-ely.5", "7136d36ef28ad8c997274811e1ad5d52d950875034c6ceb042eb6f61fbf78b0e" },
    { "6.0.52", "6.0.52-ely.5", "0eea90cdb18f1b23b8be99ebb72284c7543177a42905f1c381532f3a1a98f4d2" },
    { "6.0.54", "6.0.54-ely.5", "5be3e8a4786e9d9033d3382bb4c61c02aea7a33a615315d47e7f787549d46ac0" },
    { "6.0.55", "6.0.55-ely.5", "34603e34955ddb0fa43bdcef421f90e26d56f8681448fb5155aa31ecc4814033" },
    { "6.0.57", "6.0.57-ely.5", "f425035c708193dd646a17f6a989119fcd0499477428f5ecd859c75ec8c47087" },
    { "6.0.58", "6.0.58-ely.2", "aeef65f0d77bb5a42f4fcb57c7b5ba7c85dc7ec47c99c76f7459eac0f34be8ca" },
    { "6.0.59", "6.0.59-ely.1", "74870908a01e35a71d433a687d07710ea09f1e2dbb904be463f59de0d28d6f5d" },
    { "7.0.60", "7.0.60-ely.1", "78aa790cfe68c8afc4a3606434aef49f565863ca598e3ac63ed1d759d8df5728" },
    { "7.0.61", "7.0.61-ely.1", "5e6640c232d1f53acb21ca3dd747cef76bef0212aeee3aa1125ce4d0ffaf1b3c" },
    { "7.0.62", "7.0.62-ely.1", "1afd5996d581201151fddfe0b38cf2f4b996963b95a80de82a45f6e77dbf1d07" },
    { "7.0.63", "7.0.63-ely.1", "3085140dda5e02f7c2b546cc6c393dd6dc7d4da893e46c367dd55c3d7681a81d" },
    { "7.0.72", "7.0.72-ely.2", "9f8bd6ff7174df4b204faec5d7592f7b0126e650a2701ad526268cb6107032ae" },
    { "8.0.73", "8.0.73-ely.2", "318e2fa7cbf089b0f5a2c0af821711bfb766cc020c4e5ab12ee3ef43c76ab146" },
    { "9.0.75", "9.0.75-ely.3", "0dc8ccb56b24d0ab27a951a2c9286b994d599d77017932b4cbaf95bd293c61fa" },
    { "10.0.76", "10.0.76-ely.2", "77338718eb404088e80373cb269cebc7045d0caa264c7eb189691aaade69dc13" },
    { "10.0.77", "10.0.77-ely.2", "846830e96ec5d1dd797ba8c7ae4bc8862e240af3a360c33d6a2ffbe72462eb74" },
    { "11.0.80", "11.0.80-ely.1", "2f7f53a0b9286fae74a828182a3c9c98a1fb5f0c1b1178276d95e567403c7c1f" },
    { "12.0.81", "12.0.81-ely.1", "f2f700cc893c39b4c16f3facb6cf387a499947f654eb1e86237ac129618b3a33" },
};

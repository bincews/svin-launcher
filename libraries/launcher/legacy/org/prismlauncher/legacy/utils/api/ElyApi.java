// SPDX-License-Identifier: GPL-3.0-only

package org.prismlauncher.legacy.utils.api;

import java.io.IOException;
import java.net.Proxy;
import java.net.URL;
import java.net.URLConnection;
import java.util.regex.Pattern;

public final class ElyApi {
    private static final Pattern PLAYER_NAME = Pattern.compile("[A-Za-z0-9_]{1,16}");

    public static Texture getSkin(String player, Proxy proxy) throws IOException {
        if (!PLAYER_NAME.matcher(player).matches())
            return null;

        URL profile = new URL("https://skinsystem.ely.by/profile/" + player);
        URLConnection connection = proxy == null ? profile.openConnection() : profile.openConnection(proxy);
        connection.setConnectTimeout(6000);
        connection.setReadTimeout(6000);
        Texture skin = MojangApi.getTexture(MojangApi.getTextures(connection), "SKIN");
        if (skin == null)
            return null;

        URL texture = skin.getUrl();
        String host = texture.getHost();
        String path = texture.getPath();
        boolean ely = "ely.by".equals(host) && path.startsWith("/storage/skins/");
        boolean mojang = "textures.minecraft.net".equals(host) && path.startsWith("/texture/");
        if (!ely && !mojang)
            return null;

        return new Texture(new URL("https", host, path), skin.isSlim());
    }

    private ElyApi() {}
}

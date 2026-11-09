// SPDX-License-Identifier: GPL-3.0-only

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.nio.channels.FileChannel;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.nio.file.StandardOpenOption;
import java.security.KeyFactory;
import java.security.MessageDigest;
import java.security.Signature;
import java.security.spec.X509EncodedKeySpec;
import java.time.Duration;
import java.util.Base64;
import java.util.Comparator;
import java.util.HashSet;
import java.util.HexFormat;
import java.util.Locale;
import java.util.regex.Pattern;
import java.util.zip.ZipInputStream;

class HorizonBootstrap {
    private static final String RELEASE = "https://github.com/horizon-rust/horizon/releases/download/nightly/";
    private static final String ARCHIVE = "horizon-windows-x64.zip";
    private static final String PUBLIC_KEY = "MU4oiIQcCxkZvbwSjjXs7SySCuM6ywRQ3nGaJYZG0gw=";
    private static final String SERVER_HASH = "33680f5f2ac32864d6d7cf5e56a705fdb3e05f4c";
    private static final String MANIFEST_HASH = "11f1aefe473eefba0af62e3286c33b692807b2eb";
    private static final long MAX_ARCHIVE_BYTES = 256L * 1024 * 1024;
    private static final HttpClient HTTP = HttpClient.newBuilder()
            .followRedirects(HttpClient.Redirect.NORMAL).connectTimeout(Duration.ofSeconds(20)).build();

    public static void main(String[] args) throws Exception {
        Path root = Path.of(args[0]).toAbsolutePath();
        Files.createDirectories(root);
        try (var lock = FileChannel.open(root.resolve("download.lock"), StandardOpenOption.CREATE, StandardOpenOption.WRITE);
             var ignored = lock.lock()) {
            Path client = root.resolve("client");
            if (!Files.isRegularFile(client.resolve("horizon-client.exe"))) {
                if (!args[1].equals("online"))
                    throw new IOException("Start Horizon online once to download its client.");
                installClient(root, client);
            }
            boolean online = args[1].equals("online");
            fetchMinecraftFile(root.resolve("minecraft-26.3.json"), MANIFEST_HASH, 1024 * 1024, online,
                    "https://piston-meta.mojang.com/v1/packages/" + MANIFEST_HASH + "/26.3.json");
            fetchMinecraftFile(root.resolve("server-26.3.jar"), SERVER_HASH, MAX_ARCHIVE_BYTES, online,
                    "https://piston-data.mojang.com/v1/objects/" + SERVER_HASH + "/server.jar");
        }
    }

    private static void fetchMinecraftFile(Path file, String hash, long limit, boolean online, String url) throws Exception {
        if (Files.isRegularFile(file) && digest(file, "SHA-1").equals(hash))
            return;
        if (!online)
            throw new IOException("Start Horizon online once to prepare its resources.");
        System.out.println("Downloading " + file.getFileName() + " from Mojang…");
        Path temporary = Files.createTempFile(file.getParent(), "mojang-", ".part");
        try {
            download(url, temporary, limit);
            if (!digest(temporary, "SHA-1").equals(hash))
                throw new IOException("Minecraft checksum mismatch: " + file.getFileName());
            Files.move(temporary, file, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
        } finally {
            Files.deleteIfExists(temporary);
        }
    }

    private static void installClient(Path root, Path target) throws Exception {
        System.out.println("Downloading Horizon for Windows x64…");
        Path staging = Files.createTempDirectory(root, "download-");
        try {
            Path checksums = staging.resolve("SHA256SUMS");
            Path signature = staging.resolve("SHA256SUMS.sig");
            Path archive = staging.resolve(ARCHIVE);
            download(RELEASE + "SHA256SUMS", checksums, 65536);
            download(RELEASE + "SHA256SUMS.sig", signature, 4096);
            byte[] list = Files.readAllBytes(checksums);
            byte[] rawKey = Base64.getDecoder().decode(PUBLIC_KEY);
            byte[] key = new byte[44];
            System.arraycopy(HexFormat.of().parseHex("302a300506032b6570032100"), 0, key, 0, 12);
            System.arraycopy(rawKey, 0, key, 12, rawKey.length);
            var verifier = Signature.getInstance("Ed25519");
            verifier.initVerify(KeyFactory.getInstance("Ed25519").generatePublic(new X509EncodedKeySpec(key)));
            verifier.update("horizon release nightly\n".getBytes(StandardCharsets.US_ASCII));
            verifier.update(list);
            if (!verifier.verify(Base64.getDecoder().decode(Files.readString(signature).strip())))
                throw new IOException("Horizon release signature is invalid");
            var match = Pattern.compile("(?m)^([a-f0-9]{64})  horizon-windows-x64\\.zip\\r?$")
                    .matcher(new String(list, StandardCharsets.US_ASCII));
            if (!match.find())
                throw new IOException("The signed list lacks the Windows archive");
            download(RELEASE + ARCHIVE, archive, MAX_ARCHIVE_BYTES);
            if (!digest(archive, "SHA-256").equals(match.group(1)))
                throw new IOException("Horizon archive checksum mismatch; retry if Nightly just changed");
            Path extracted = staging.resolve("payload");
            unpack(archive, extracted);
            for (String name : new String[]{"horizon-client.exe", "horizon-prepare.exe", "research-lock.json", "exporters/manifest.json"}) {
                if (!Files.isRegularFile(extracted.resolve(name)))
                    throw new IOException("Horizon archive is missing " + name);
            }
            Files.move(extracted, target, StandardCopyOption.ATOMIC_MOVE);
        } finally {
            try (var paths = Files.walk(staging)) {
                for (Path path : paths.sorted(Comparator.reverseOrder()).toList())
                    Files.deleteIfExists(path);
            }
        }
    }

    private static void download(String url, Path target, long limit) throws Exception {
        var request = HttpRequest.newBuilder(URI.create(url)).timeout(Duration.ofMinutes(4)).GET().build();
        var response = HTTP.send(request, HttpResponse.BodyHandlers.ofInputStream());
        try (var input = response.body()) {
            if (response.statusCode() != 200)
                throw new IOException("Download returned HTTP " + response.statusCode() + ": " + request.uri().getPath());
            try (var output = Files.newOutputStream(target)) {
                byte[] data = new byte[65536];
                long size = 0;
                for (int count; (count = input.read(data)) >= 0;) {
                    size += count;
                    if (size > limit)
                        throw new IOException("Download exceeded its size limit");
                    output.write(data, 0, count);
                }
            }
        }
    }

    private static String digest(Path path, String algorithm) throws Exception {
        var digest = MessageDigest.getInstance(algorithm);
        try (var input = Files.newInputStream(path)) {
            byte[] bytes = new byte[65536];
            for (int count; (count = input.read(bytes)) >= 0;)
                digest.update(bytes, 0, count);
        }
        return HexFormat.of().formatHex(digest.digest());
    }

    private static void unpack(Path archive, Path target) throws IOException {
        String prefix = "horizon-windows-x64/";
        var entries = new HashSet<String>();
        Files.createDirectory(target);
        try (var zip = new ZipInputStream(Files.newInputStream(archive))) {
            byte[] data = new byte[65536];
            long total = 0;
            for (var entry = zip.getNextEntry(); entry != null; entry = zip.getNextEntry()) {
                String name = entry.getName();
                if (name.equals(prefix) && entry.isDirectory())
                    continue;
                if (!name.startsWith(prefix) || name.contains(":") || name.contains("\\"))
                    throw new IOException("Unexpected Horizon archive entry: " + name);
                String relative = name.substring(prefix.length());
                for (String part : relative.split("/")) {
                    if (part.isEmpty() || part.equals(".") || part.equals("..") || part.endsWith(" ") || part.endsWith("."))
                        throw new IOException("Invalid Horizon archive entry: " + name);
                }
                if (!entries.add(relative.toLowerCase(Locale.ROOT)))
                    throw new IOException("Duplicate Horizon archive entry: " + name);
                Path destination = target.resolve(relative).normalize();
                if (!destination.startsWith(target) || destination.equals(target))
                    throw new IOException("Archive entry escapes the Horizon directory");
                if (entry.isDirectory()) {
                    Files.createDirectories(destination);
                    continue;
                }
                Files.createDirectories(destination.getParent());
                try (var output = Files.newOutputStream(destination, StandardOpenOption.CREATE_NEW)) {
                    for (int count; (count = zip.read(data)) >= 0;) {
                        total += count;
                        if (total > 1024L * 1024 * 1024)
                            throw new IOException("Horizon archive exceeded its expanded size limit");
                        output.write(data, 0, count);
                    }
                }
            }
        }
    }
}

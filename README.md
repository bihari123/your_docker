## Your Docker

A basic implementaion of docker that can download the image from the docker hub and then lets you run it locally with filesystem isolation and process isolation .

### Run the project

```
./your_docker.sh run alpine /bin/sh -c '/bin/ls /'

```

### Comments

The official Docker implementation now uses pivot_root instead of chroot, for reasons listed here.( https://tbhaxor.com/pivot-root-vs-chroot-for-containers/)

On macOS with Apple Silicon Macs, mounting /proc file system inside chrooted directory is mandatory. Otherwise running /usr/local/bin/docker-explorer binary will result in following error:

rosetta error: Unable to open /proc/self/exe: 2

```
Docker API can be very confusing:

You need to get an auth token, but you don't need a username/password
Say your image is busybox/latest, you would make a GET request to this URL: https://auth.docker.io/token?service=registry.docker.io&scope=repository:library/busybox:pull
You'll send the token you receive in following API calls as a header: "Authorization = Bearer <token>"
Next step is to get the manifest. The URL to get the manifest will be: https://registry.hub.docker.com/v2/library/busybox/manifests/latest
I'd recommend using "Accept = application/vnd.docker.distribution.manifest.v2+json" header to get v2 of this response, which tells the media type for each layer
The last step is to download the layer and extract the files. The URL for the API call will be: https://registry.hub.docker.com/v2/library/busybox/blobs/<sha256:xxxx>
```

### StackOverflow

1. https://superuser.com/questions/1737519/why-a-fork-is-often-followed-by-an-exec
2. https://stackoverflow.com/questions/54152633/creating-a-child-process-without-fork
3. https://cs.brown.edu/courses/csci0300/2022/notes/l17.html
4. https://en.wikipedia.org/wiki/Chroot
5. https://man7.org/linux/man-pages/man7/pid_namespaces.7.html
6. https://tbhaxor.com/pivot-root-vs-chroot-for-containers/
7. https://www.youtube.com/watch?v=sK5i-N34im8
8. https://itnext.io/container-runtime-in-rust-part-0-7af709415cda

#include <glovebox.h>
#include <rpc/client.h>

thread_local ClientParams client_params;

using Q4_4 = Fixed<4, 4>;

int main() {
	ClientKey key = read_client_key("secret.key");
	if (key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	client_params = ClientParams(key);

	double plaintext_xs[] = {1.0, 1.5};
	puts("Constructing values...");
	Array<Q4_4, 2> xs(false);
	for (int i = 0; i < 2; i++)
		xs.put(plaintext_xs[i], i);

	puts("Connecting to server...");
	rpc::client client("127.0.0.1", 8000);
	puts("Awaiting result...");
	Array<Q4_4, 2> ys =
	    client.call("polyeval", xs.serialize()).as<std::string>();
	for (int i = 0; i < 2; i++) {
		Q4_4 y;
		ys.get(y, i);
		printf("p(%lf) = %lf\n", plaintext_xs[i], y.toDouble());
	}

	return 0;
}

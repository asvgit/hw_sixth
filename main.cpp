#include <iostream>
#include <cassert>
#include <sstream>
#include <vector>
#include <unistd.h>

using string = std::string;
using StringVector = std::vector<string>;

class BulkManager {
public:
	class Observer {
	public:
		class UpdateHandler {
		public:
			virtual void Update(Observer *, const string &) = 0;
		};

		Observer(BulkManager *mgr, const int size)
			: max_size(size) { mgr->Subscribe(this); }
		void SetUpdadeHandler(UpdateHandler *uh) { m_update_handler = uh; };
		StringVector& GetBulk() { return m_bulk; }
		int GetMaxSize() { return max_size; }
		virtual void PostBulk() = 0;
		void Update(const string &msg) {
			if (m_update_handler != nullptr)
				m_update_handler->Update(this, msg);
		}
	private:
		UpdateHandler *m_update_handler = nullptr;
	protected:
		StringVector m_bulk;
		const int max_size;
	};

	void Subscribe(Observer *obs) { m_subs.push_back(obs); }

	void Listen() {
		for (string line; std::getline(std::cin, line);)
			Notify(line);
	}

private:
	std::vector<Observer*> m_subs;

	void Notify(const string &chunk) {
		for (const auto &s : m_subs)
			s->Update(chunk);
	};
};

class DynamicHandler : public BulkManager::Observer::UpdateHandler {
	int m_count = 0;
	virtual void Update(BulkManager::Observer *, const string &) override;
};

class SizedHandler : public BulkManager::Observer::UpdateHandler {
	virtual void Update(BulkManager::Observer *o, const string &cmd) override {
		auto &bulk = o->GetBulk();
		if (cmd == "{") {
			o->PostBulk();
			bulk.clear();
			o->SetUpdadeHandler(new DynamicHandler());
			return;
		}

		bulk.push_back(cmd);
		if (bulk.size() >= o->GetMaxSize()) {
			o->PostBulk();
			bulk.clear();
		}
	}
};

void DynamicHandler::Update(BulkManager::Observer *o, const string &cmd) {
	if (cmd == "{") {
		++m_count;
		return;
	}
	if (m_count && cmd == "}") {
		--m_count;
		return;
	}

	auto &bulk = o->GetBulk();
	if (!m_count && cmd == "}") {
		o->PostBulk();
		bulk.clear();
		o->SetUpdadeHandler(new SizedHandler());
	} else
		bulk.push_back(cmd);
}

class ConsoleOutput : public BulkManager::Observer {
public:

	ConsoleOutput(BulkManager *mgr, const int size)
		: Observer(mgr, size) {};

	void PostBulk() override {
		if (m_bulk.empty())
			return;
		sleep(1);
		for (auto &n : m_bulk) {
			std::cout << (&n == &m_bulk.front() ? "bulk: " : ", ")
					<< n;
		}
		std::cout << std::endl;
	}
};

int main(int argc, char *argv[]) {
	try {
		assert(argc == 2);
		const int bulk_size = [&]() {
			std::stringstream ss(argv[1]);
			int n;
			ss >> n;
			return n;
		}();

		BulkManager bulk_mgr;
		ConsoleOutput co(&bulk_mgr, bulk_size);
		co.SetUpdadeHandler(new SizedHandler());
		bulk_mgr.Listen();
	} catch(const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}

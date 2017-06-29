#pragma once

#include <thread>
#include <vector>
#include <atomic>
#include <iostream>
#include <condition_variable>


template <typename Data, typename Value>
class ThreadPool
{
	typedef std::function<Value(Data)> Call;
	typedef std::unique_lock<std::mutex> Lock;

public:
	ThreadPool(const ThreadPool& cpy);

	ThreadPool(int threadCount);
	~ThreadPool();

	std::vector<Value> run(Call call, const std::vector<Data> &data);

private:

	void waitForData();
	void display(std::string str);

	bool isKill;

	Call _call;

	std::mutex _startedCounterMutex;
	int _startedCounter;
	std::mutex _finishedCounterMutex;
	int _finishedCounter;

	std::mutex _condMutexFinished;
	std::mutex _condMutexStarted;
	std::condition_variable _finished;
	std::condition_variable _started;

	std::mutex _displayMutex;

	std::vector<Data> _data;
	std::vector<Value> _values;
	std::vector<std::thread*> _threads;
};

template <typename Data, typename Value>
ThreadPool<Data, Value>::ThreadPool(int threadCount)
{
	isKill = false;

	_threads.resize(threadCount);
	for (int i = 0; i < threadCount; i++)
	{
		auto functor = std::bind(&ThreadPool::waitForData, this);
		_threads[i] = new std::thread(functor);
	}

}

template <typename Data, typename Value>
ThreadPool<Data, Value>::~ThreadPool()
{
	isKill = true;

	_started.notify_all();

	for (size_t i = 0; i < _threads.size(); i++)
	{
		_threads[i]->join();
		delete _threads[i];
	}
}

template <typename Data, typename Value>
void ThreadPool<Data, Value>::waitForData()
{
	while (1)
	{
		{
			Lock lock(_condMutexStarted);
			_started.wait(lock);
		}
		if (isKill)
			return;
		{
			Lock lock(_finishedCounterMutex);
			_finishedCounter++;
		}

		while (1)
		{
			int index;
			{
				Lock lock(_startedCounterMutex);
				index = _startedCounter;
				_startedCounter++;
			}

			if (static_cast<size_t>(index) < _data.size())
				_values[index] = _call(_data[index]);
			else
			{
				{
					Lock lock(_finishedCounterMutex);
					_finishedCounter--;
					if (_finishedCounter <= 0)
						_finished.notify_one();
				}
				break;
			}
		}
	}
}

template <typename Data, typename Value>
std::vector<Value> ThreadPool<Data, Value>::run(Call call, const std::vector<Data> &data)
{
	_call = call;
	_data = data;

	_values.resize(_data.size());
	_startedCounter = 0;
	_finishedCounter = 0;
	_started.notify_all();
	{
		Lock lock(_condMutexFinished);
		_finished.wait(lock);
	}
	return (_values);
}
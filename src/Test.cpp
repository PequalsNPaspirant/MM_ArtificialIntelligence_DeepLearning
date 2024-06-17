#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <numeric>
using namespace std;

//#include "Utils/Utils_MM_Assert.h"
#include "DynamicProgramming/DP_Common.h"
#include "MM_UnitTestFramework/MM_UnitTestFramework.h"

namespace mm {

	// ============================= Dynamic Programing Approach =============================

	void reverse(std::vector<int>& vec, int start, int end)
	{
		std::reverse(vec.begin() + start, vec.begin() + end + 1);
	}

	void applySteps(std::vector<int>& vec, const std::vector<std::pair<int, int>>& steps)
	{
		for (const std::pair<int, int>& curStep : steps)
		{
			reverse(vec, curStep.first, curStep.second);
		}
	}

	bool isSorted(const vector<int>& vec)
	{
		for (int i = 1; i < vec.size(); ++i)
			if (vec[i - 1] > vec[i])
				return false;

		return true;
	}

	bool isSorted(const vector<int>& vec, int start, int end)
	{
		for (int i = start + 1; i <= end; ++i)
			if (vec[i - 1] > vec[i])
				return false;

		return true;
	}

	namespace {
		template<typename T>
		void hash_combine(std::size_t& seed, const T& value)
		{
			seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	}

	/*
	Time Complexity  : number of distinct subproblems * Complexity of each subproblem
	Time Complexity  : O(m * n)
	Space Complexity : O(m * n)
	The recursive call stack would grow with O(m + n)
	*/

	struct Key
	{
		Key(const std::vector<int>& vec) :
			vec_{ vec }
		{}

		std::vector<int> vec_;

		struct Hasher
		{
			size_t operator()(const Key& key) const
			{
				std::size_t seed = 0;
				for (int num : key.vec_)
					hash_combine(seed, num);

				return seed;
			}
		};

		struct Comparator
		{
			bool operator()(const Key& lhs, const Key& rhs) const
			{
				return lhs.vec_ == rhs.vec_;
			}
		};
	};

	std::unordered_map<Key, std::vector<std::pair<int, int>>, Key::Hasher, Key::Comparator> cache;
	std::unordered_map<Key, bool, Key::Hasher, Key::Comparator> visitedMap;

	std::vector<std::pair<int, int>> getMinStepsToSortArrayUsingRangeReverse_top_down_v1(vector<int>& vec)
	{
		if (isSorted(vec))
			return std::vector<std::pair<int, int>>{};

		bool& visited = visitedMap[vec];
		if(visited)
			return std::vector<std::pair<int, int>>{ std::make_pair(-1, -1) };

		visited = true;
		std::vector<std::pair<int, int>>& steps = cache[vec];
		if (steps.size() == 0)
		{
			std::vector<std::pair<int, int>> minSteps;
			for (int start = 0; start < vec.size() - 1; ++start)
			{
				for (int end = start + 1; end < vec.size(); ++end)
				{
					reverse(vec, start, end);

					std::vector<std::pair<int, int>> tempSteps = getMinStepsToSortArrayUsingRangeReverse_top_down_v1(vec);
					bool invalid = (tempSteps.size() == 1 && tempSteps[0].first == -1);
					if (!invalid)
					{
						tempSteps.insert(tempSteps.begin(), std::make_pair(start, end));
						//tempSteps.push_back(std::make_pair(start, end));
						if (minSteps.empty() || minSteps.size() > tempSteps.size())
							minSteps = tempSteps;
					}

					reverse(vec, start, end);
				}
			}
			steps = minSteps;
		}

		return steps;
	}

	struct Data
	{
		Data(const std::vector<std::pair<int, int>>& vecSteps, const std::vector<int>& vec) :
			vecSteps_{ vecSteps }, vec_{ vec }
		{}

		std::vector<std::pair<int, int>> vecSteps_;
		std::vector<int> vec_;
		int totalSwaps_{ 0 };

		struct Comparator_v1
		{
			bool operator()(const Data& lhs, const Data& rhs) const
			{
				return lhs.vecSteps_.size() > rhs.vecSteps_.size();
			}
		};

		struct Comparator_v2
		{
			bool operator()(const Data& lhs, const Data& rhs) const
			{
				return lhs.totalSwaps_ > rhs.totalSwaps_;
			}
		};
	};

	std::vector<Data> heap;

	std::vector<std::pair<int, int>> getMinStepsToSortArrayUsingRangeReverse_top_down_v2(const vector<int>& vec)
	{
		heap.clear();
		visitedMap.clear();
		heap.push_back(Data{ std::vector<std::pair<int, int>>{}, vec });
		std::make_heap(heap.begin(), heap.end(), Data::Comparator_v1{});

		while (!heap.empty())
		{
			std::pop_heap(heap.begin(), heap.end(), Data::Comparator_v1{});
			Data minValue = heap.back();
			heap.pop_back();
			visitedMap.erase(minValue.vec_);

			if (isSorted(minValue.vec_))
				return minValue.vecSteps_;

			for (int start = 0; start < minValue.vec_.size() - 1; ++start)
			{
				for (int end = start + 1; end < minValue.vec_.size(); ++end)
				{
					Data minValueCopy = minValue;
					reverse(minValueCopy.vec_, start, end);
					bool& visited = visitedMap[minValueCopy.vec_];
					if (visited)
						continue;

					visited = true;
					minValueCopy.vecSteps_.push_back(std::make_pair(start, end));
					heap.push_back(minValueCopy);
					std::push_heap(heap.begin(), heap.end(), Data::Comparator_v1{});
				}
			}
		}

		return std::vector<std::pair<int, int>>{};
	}

	std::vector<std::pair<int, int>> getMinStepsToSortArrayUsingRangeReverse_top_down_v3(const vector<int>& vec)
	{
		heap.clear();
		visitedMap.clear();
		heap.push_back(Data{ std::vector<std::pair<int, int>>{}, vec });
		std::make_heap(heap.begin(), heap.end(), Data::Comparator_v1{});

		while (!heap.empty())
		{
			std::pop_heap(heap.begin(), heap.end(), Data::Comparator_v2{});
			Data minValue = heap.back();
			heap.pop_back();
			visitedMap.erase(minValue.vec_);

			if (isSorted(minValue.vec_))
				return minValue.vecSteps_;

			for (int start = 0; start < minValue.vec_.size() - 1; ++start)
			{
				for (int end = start + 1; end < minValue.vec_.size(); ++end)
				{
					Data minValueCopy = minValue;
					reverse(minValueCopy.vec_, start, end);
					bool& visited = visitedMap[minValueCopy.vec_];
					if (visited)
						continue;

					visited = true;
					minValueCopy.vecSteps_.push_back(std::make_pair(start, end));
					minValueCopy.totalSwaps_ += ((end - start) / 2);
					heap.push_back(minValueCopy);
					std::push_heap(heap.begin(), heap.end(), Data::Comparator_v1{});
				}
			}
		}

		return std::vector<std::pair<int, int>>{};
	}

	std::vector<std::pair<int, int>> getMinStepsToSortArrayUsingRangeReverse_bottom_up_v1(const std::vector<int>& vec)
	{
		std::vector<int> stepCount(vec.size(), 0);
		std::vector<std::vector<std::pair<int, int>>> steps(vec.size(), std::vector<std::pair<int, int>>{});
		std::vector<vector<int>> sortedVecVec(vec.size(), vector<int>{});
		
		sortedVecVec[0] = vec;
		for (int i = 1; i < vec.size(); ++i)
		{
			sortedVecVec[i] = sortedVecVec[i - 1];
			std::vector<int>& sortedVec = sortedVecVec[i];
			steps[i] = steps[i - 1];
			int j = 0;
			for (; j < i; ++j)
			{
				if (sortedVec[i] <= sortedVec[j])
					break;
			}
			if (j <= (i - 2))
			{
				stepCount[i] = stepCount[i - 1] + 2;
				steps[i].push_back(std::make_pair(j, i));
				steps[i].push_back(std::make_pair(j + 1, i));
				reverse(sortedVec, j, i);
				reverse(sortedVec, j + 1, i);
			}
			else if (j == (i - 1))
			{
				stepCount[i] = stepCount[i - 1] + 1;
				steps[i].push_back(std::make_pair(j, i));
				reverse(sortedVec, j, i);
			}
			else if (j == i)
			{
				stepCount[i] = stepCount[i - 1] + 0;
			}
			
			for (int j = 0; j < i; ++j)
			{
				std::vector<int>& curSortedVec = sortedVecVec[j];
				for (int k = 0; k < i; ++k)
				{
					reverse(curSortedVec, k, i);
					if (isSorted(curSortedVec, 0, i))
					{
						int curSteps = stepCount[j] + 1;
						if (stepCount[i] > curSteps)
						{
							stepCount[i] = curSteps;
							steps[i] = steps[j];
							steps[i].push_back(std::make_pair(k, i));
							sortedVecVec[i] = curSortedVec;
						}
					}

					reverse(curSortedVec, k, i);
				}
			}
		}

		return steps.back();
	}

	// ============================= Testing =============================

	MM_DECLARE_FLAG(DP_MinStepsToSortArrayUsingRangeReverse);

	struct MinStepsToSortArrayUsingRangeReverseTestData
	{
		std::vector<int> vec_;
		std::vector<std::pair<int, int>> expectedSteps_;
	};

	template<typename Fun, typename... Args>
	void DP_MinStepsToSortArrayUsingRangeReverseHelper(const std::string& algoName, const MinStepsToSortArrayUsingRangeReverseTestData& testData, Fun fun, Args... args)
	{
		std::cout << "\n" << algoName << ": testing input of size: " << testData.vec_.size() << std::endl;
		std::vector<int> vec = testData.vec_;
		std::vector<std::pair<int, int>> steps = fun(vec);
		MM_EXPECT_TRUE(vec == testData.vec_, vec, testData.vec_);
		MM_EXPECT_TRUE((steps.size() <= testData.expectedSteps_.size()), steps, testData.expectedSteps_);
		applySteps(vec, steps);
		const bool sorted = isSorted(vec);
		MM_EXPECT_TRUE(sorted, sorted, testData.vec_, steps, vec);
	}

	MM_UNIT_TEST(DP_MinStepsToSortArrayUsingRangeReverse_test_1, DP_MinStepsToSortArrayUsingRangeReverse)
	{
		//failed test cases
		//const std::vector<int> vec = { 5, 1, 3, 6, 2, 4 };
		//std::vector<std::pair<int, int>> temp = getMinStepsToSortArrayUsingRangeReverse_bottom_up_v1(vec);

		vector<MinStepsToSortArrayUsingRangeReverseTestData> vecTestData;
		std::vector<int> testDataSizes{ 2,3,4,5,6,7,8,9,10,15,20,50,100 };
		constexpr const int numTestCasesOfEach = 10;
		for (int size : testDataSizes)
		{
			for (int i = 0; i < numTestCasesOfEach; ++i)
			{
				std::vector<int> vec(size);
				std::iota(vec.begin(), vec.end(), 1);
				std::vector<std::pair<int, int>> steps;

				int n = size - 1;
				int numMaxReverseRanges = n * (n + 1) * (2 * n + 1) / 6;
				while (steps.size() < std::min(numMaxReverseRanges, size * 2))
				{
					int minStartIndex = 0;
					int maxStartIndex = size - 2;
					int start = minStartIndex + rand() % (maxStartIndex - minStartIndex + 1);
					int minEndIndex = start + 1;
					int maxEndIndex = size - 1;
					int end = minEndIndex + rand() % (maxEndIndex - minEndIndex + 1);
					std::pair<int, int> curPair = std::make_pair(start, end);
					if (steps.empty() || steps.back() != curPair)
					{
						reverse(vec, start, end);
						steps.push_back(curPair);
					}
				}

				vecTestData.push_back(MinStepsToSortArrayUsingRangeReverseTestData{ vec, steps });
			}
		}
		
		for (int i = 0; i < vecTestData.size(); ++i)
		{
			if (vecTestData[i].vec_.size() < 7)
			{
				visitedMap.clear();
				DP_MinStepsToSortArrayUsingRangeReverseHelper("top_down_v1", vecTestData[i], getMinStepsToSortArrayUsingRangeReverse_top_down_v1);
			}
			if (vecTestData[i].vec_.size() < 10)
				DP_MinStepsToSortArrayUsingRangeReverseHelper("top_down_v2", vecTestData[i], getMinStepsToSortArrayUsingRangeReverse_top_down_v2);
			if(vecTestData[i].vec_.size() < 5)
				DP_MinStepsToSortArrayUsingRangeReverseHelper("top_down_v3", vecTestData[i], getMinStepsToSortArrayUsingRangeReverse_top_down_v3);
			DP_MinStepsToSortArrayUsingRangeReverseHelper("bottom_up_v1", vecTestData[i], getMinStepsToSortArrayUsingRangeReverse_bottom_up_v1);
		}
	}
}

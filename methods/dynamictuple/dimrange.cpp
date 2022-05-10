// #include "dimrange.h"

// using namespace std;

// uint64_t dim_check_tuple_cost = 5;  // us
// // uint64_t check_group_cost = 2;
// uint64_t dim_check_rule_cost = 5;

// int dim_prefix_len[5] = {32, 32, 16, 16, 8};

// DimsRange InitDimsRange() {
// 	DimsRange dims_range;
// 	int step = 8;
// 	for (int i = 0; i < 5; ++i)
// 		for (int j = 0; j <= dim_prefix_len[i]; j += step) {
// 			int k = min(j + step, dim_prefix_len[i]);
// 			DimRange dim_range;
// 			dim_range.x1 = j;
// 			dim_range.x2 = k;
// 			dims_range.dims[i].push_back(dim_range);
// 			for (int p = j; p <= k; ++p)
// 				dims_range.reduced[i][p] = j;
// 		}
// 	return dims_range;
// }

// bool CmpDimRulePriority(DimRule *rule1, DimRule *rule2) {
// 	return rule1->priority > rule2->priority;
// }

// int cmp_dim;

// bool CmpDimRulePrefixLen(DimRule *rule1, DimRule *rule2) {
// 	return rule1->prefix_len[cmp_dim] < rule2->prefix_len[cmp_dim];
// }

// bool CmpDimRuleReducedKey(DimRule *rule1, DimRule *rule2) {
// 	for (int i = 0; i < 5; ++i)
// 		if (rule1->reduced_key[i] != rule2->reduced_key[i])
// 			return rule1->reduced_key[i] < rule2->reduced_key[i];
// 	return rule1->priority > rule2->priority;
// }

// bool SameReducedKey(DimRule *rule1, DimRule *rule2) {
// 	for (int i = 0; i < 5; ++i)
// 		if (rule1->reduced_key[i] != rule2->reduced_key[i])
// 			return false;
// 	return true;
// }

// vector<DimRule*> GetDimRules(vector<Rule*> &rules) {
// 	vector<DimRule*> dim_rules;
// 	int rules_num = rules.size();
// 	// printf("rules_num %d\n", rules_num);
// 	for (int i = 0; i < rules_num; ++i) {
// 		DimRule *dim_rule = (DimRule*)malloc(sizeof(DimRule));
// 		for (int j = 0; j < 5; ++j) {
// 			dim_rule->key[j] = rules[i]->range[j][0];
// 			dim_rule->prefix_len[j] = rules[i]->prefix_len[j];
// 			dim_rule->priority = rules[i]->priority;
// 		}
// 		dim_rules.push_back(dim_rule);
// 	}
// 	sort(dim_rules.begin(), dim_rules.end(), CmpDimRulePriority);
// 	for (int i = 0; i < rules_num; ++i)
// 		dim_rules[i]->priority = rules_num - i;
// 	return dim_rules;
// }

// int dim_len_rules_num[33];

// int dim_check_tuple_num[33];

// uint32_t GetPrefixLen(DimRule *rule, DimsRange &dims_range, int except_dim) {
// 	uint32_t prefix_len = 0;
// 	for (int i = 0; i < 5; ++i) {
// 		prefix_len <<= 6;
// 		if (i != except_dim)
// 			prefix_len |= dims_range.reduced[i][rule->prefix_len[i]];
// 	}
// 	return prefix_len;
// }

// void PrintPrefixLen(uint32_t prefix_len) {
// 	char lens[5];
// 	for (int i = 4; i >= 0; --i) {
// 		lens[i] = prefix_len & 0x3F;
// 		prefix_len >>= 6;
// 	}
// 	for (int i = 0; i < 5; ++i)
// 		printf("%d ", lens[i]);
// 	printf("\n");
// }

// DimKey GetReducedDimKey(DimRule *rule) {
// 	DimKey dim_key;
// 	for (int j = 0; j < 5; ++j)
// 		dim_key.key[j] = rule->reduced_key[j];
// 	return dim_key;
// }

// map<uint32_t, uint32_t> tuples_priority;
// uint64_t tuples_priority_sum;
// uint64_t dim_check_tuples_num[33][33];
// uint64_t dim_check_rules_num[33][33];
// uint64_t tuple_cost_sum[33][33];
// uint64_t dp_cost[33][33];
// uint32_t dp_tuple[33][33];
// uint32_t dp_rule[33][33];

// map<uint32_t, map<DimKey, uint32_t>> tuples_key[33];

// void CalculateDimCost(vector<DimRule*> rules, DimsRange &dims_range, int dim) {
// 	int rules_num = rules.size();
// 	sort(rules.begin(), rules.end(), CmpDimRulePriority);

// 	tuples_priority.clear();
// 	tuples_key[0].clear();
// 	uint64_t check_tuples_num = 0;
// 	uint64_t check_rules_num = 0;
// 	memset(dim_check_rules_num, 0, sizeof(dim_check_rules_num));
// 	for (int i = 0; i < rules_num; ++i)  {
// 		for (int j = 0; j < 5; ++j) {
// 			int k = dim_prefix_len[j] - dims_range.reduced[j][rules[i]->prefix_len[j]];
// 			rules[i]->reduced_key[j] = (uint64_t)rules[i]->key[j] >> k << k;
// 		}
// 		uint32_t prefix_len = GetPrefixLen(rules[i], dims_range, -1);
// 		DimKey dim_key = GetReducedDimKey(rules[i]);
// 		if (rules[i]->priority > tuples_priority[prefix_len]) {
// 			check_tuples_num += rules[i]->priority - tuples_priority[prefix_len];
// 			tuples_priority[prefix_len] = rules[i]->priority;
// 		}
// 		check_rules_num += ++tuples_key[0][prefix_len][dim_key];

// 		int k = dims_range.reduced[dim][rules[i]->prefix_len[dim]];
// 		dim_check_rules_num[0][k] += tuples_key[0][prefix_len][dim_key];
// 	}
// 	uint64_t cost = check_tuples_num * dim_check_tuple_cost +
// 					check_rules_num * dim_check_rule_cost;
// 	printf("cost %lu : tuple %lu rule %lu\n", cost, check_tuples_num, check_rules_num);
// 	for (int i =0; i < 5; ++i) {
// 		for (int j = 0; j <= dim_prefix_len[i]; ++j)
// 			printf("%d ", dims_range.reduced[i][j]);
// 		printf("\n");
// 	}
// 	// for (int i = 0; i <= dim_prefix_len[dim]; ++i)
// 	// 	printf("%d ", dim_check_rules_num[0][i]);
// 	// printf("\n");

// }

// void CalculateDimRangeX(vector<DimRule*> rules, DimsRange &dims_range, int dim, int x) {
// 	int rules_num = rules.size();
// 	for (int i = 0; i < rules_num; ++i) 
// 		if (rules[i]->prefix_len[dim] >= x) 
// 			for (int j = 0; j < 5; ++j) {
// 				int k;
// 				if (j == dim)
// 					k = dim_prefix_len[j] - x;
// 				else
// 					k = dim_prefix_len[j] - dims_range.reduced[j][rules[i]->prefix_len[j]];
// 				// int k = dim_prefix_len[dim] - x;
// 				rules[i]->reduced_key[j] = (uint64_t)rules[i]->key[j] >> k << k;
// 			}

// 	cmp_dim = dim;
// 	sort(rules.begin(), rules.end(), CmpDimRulePrefixLen);
// 	tuples_priority.clear();
// 	tuples_priority_sum = 0;
// 	memset(dim_check_tuples_num[x], 0, sizeof(dim_check_tuples_num[x]));
// 	for (int i = 0; i < rules_num; ++i)
// 		if (rules[i]->prefix_len[dim] >= x) {
// 			uint32_t prefix_len = GetPrefixLen(rules[i], dims_range, dim);
// 			// PrintPrefixLen(prefix_len);
// 			if (rules[i]->priority > tuples_priority[prefix_len]) {
// 				tuples_priority_sum += rules[i]->priority - tuples_priority[prefix_len];
// 				// printf("i %d priority %d sum %llu\n", i, rules[i]->priority, tuples_priority_sum);
// 				// printf("tuples_priority %llu, %llu\n", tuples_priority[prefix_len], tuples_priority[prefix_len] - rules[i]->priority);
// 				// PrintPrefixLen(prefix_len);
// 				tuples_priority[prefix_len] = rules[i]->priority;
// 			}
// 			dim_check_tuples_num[x][rules[i]->prefix_len[dim]] = tuples_priority_sum;
// 		}
// 	for (int i = x + 1; i <= dim_prefix_len[dim]; ++i)
// 		dim_check_tuples_num[x][i] = max(dim_check_tuples_num[x][i], dim_check_tuples_num[x][i - 1]);
// 	// for (int i = x; i <= dim_prefix_len[dim]; ++i)
// 	// 	printf("dim_check_tuples_num %d : %llu\n", i, dim_check_tuples_num[i]);
// 	// printf("tuples_priority.size() %d\n", tuples_priority.size());


// 	// Rule
// 	sort(rules.begin(), rules.end(), CmpDimRulePriority);
// 	for (int i = 0; i <= dim_prefix_len[dim]; ++i) {
// 		tuples_key[i].clear();
// 		// printf("i %d size %d \n", i, tuples_key[i].size());
// 	}
// 	// printf("\n");
// 	memset(dim_check_rules_num[x], 0, sizeof(dim_check_rules_num[x]));
// 	FILE *fp = fopen("output1", "a+");
// 	for (int i = 0; i < rules_num; ++i)
// 		if (rules[i]->prefix_len[dim] >= x) {
// 			uint32_t prefix_len = GetPrefixLen(rules[i], dims_range, dim);
// 			DimKey dim_key = GetReducedDimKey(rules[i]);
// 			for (int k = rules[i]->prefix_len[dim]; k <= dim_prefix_len[dim]; ++k) {
// 				int num = ++tuples_key[k][prefix_len][dim_key];
// 				// if (i == 0 && num >= 2) {
// 				// 	printf("Wrong x = %d k = %d prefix_len = %d\n", x, k, rules[i]->prefix_len[dim]);
// 				// 	exit(1);
// 				// }
// 				dim_check_rules_num[x][k] += num;
// 				// if (x == 15 && k == 32) fprintf(fp, "%d : num %d sum %d\n", i, num, dim_check_rules_num[x][k]);
// 				// printf("%d %d -- %d\n", x, rules[i]->prefix_len[dim], dim_check_rules_num[x][rules[i]->prefix_len[dim]] );
// 			}
// 		}
// 	fclose(fp);
// 	// if (x == 7) {
// 	// 	printf("dim_check_rules_num %d\n", x);
// 	// 	for (int i = 0; i <= dim_prefix_len[dim]; ++i)
// 	// 		printf("%llu ", dim_check_rules_num[x][i]);
// 	// 	printf("\n");
// 	// }

// 	// for (int i = x; i <= dim_prefix_len[dim]; ++i)
// 	// 	printf("dim_check_rules_num %d : %llu\n", i, dim_check_rules_num[i]);


// 	for (int i = x; i <= dim_prefix_len[dim]; ++i)
// 		tuple_cost_sum[x][i] = dim_check_tuples_num[x][i] * dim_check_tuple_cost +
// 							   dim_check_rules_num[x][i] * dim_check_rule_cost;
// }

// void GetDpRange(int i, int j, DimsRange &dims_range, int dim) {
// 	if (dp_cost[i][j] == tuple_cost_sum[i][j]) {
// 		// printf("%d %d\n", i, j);
// 		for (int k = i; k <= j; ++k)
// 			dims_range.reduced[dim][k] = i;
// 		return;
// 	}
// 	// printf("- %d %d\n", i, j);
// 	for (int k = i; k < j; ++k)
// 		if (dp_cost[i][j] == dp_cost[i][k] + dp_cost[k + 1][j]) {
// 			// printf("--- %d %d\n", i, k);
// 			// printf("--- %d %d\n", k + 1, j);
// 			GetDpRange(i, k, dims_range, dim);
// 			GetDpRange(k + 1, j, dims_range, dim);
// 			break;
// 		}
// }

// void DynamicProgrammingX(int dim) {
// 	int len = dim_prefix_len[dim];
// 	for (int p = 0; p <= len; ++p)
// 		for (int i = 0; i + p <= len; ++i) {
// 			int j = i + p;
// 			dp_cost[i][j] = tuple_cost_sum[i][j];
// 			dp_tuple[i][j] = dim_check_tuples_num[i][j];
// 			dp_rule[i][j] = dim_check_rules_num[i][j];
// 			for (int k = i; k < j; ++k)
// 				if (dp_cost[i][j] > dp_cost[i][k] + dp_cost[k + 1][j]) {
// 					dp_cost[i][j] = dp_cost[i][k] + dp_cost[k + 1][j];
// 					dp_tuple[i][j] = dp_tuple[i][k] + dp_tuple[k + 1][j];
// 					dp_rule[i][j] = dp_rule[i][k] + dp_rule[k + 1][j];
// 				}
// 		}
// 	// for (int i = 0; i < dim_prefix_len[dim]; ++i) {
// 	// 	for (int j = 0; j < dim_prefix_len[dim]; ++j)
// 	// 		printf("%llu ", dp_cost[i][j]);
// 	// 	printf("\n");
// 	// }
// 	// printf("dp_cost %llu : tuple %llu rule %llu\n", dp_cost[0][len], dp_tuple[0][len], dp_rule[0][len]);
// 	// printf("dp_cost %llu\n", dp_cost[0][32]);
// }

// uint64_t CalculateDimRange(vector<DimRule*> rules, DimsRange &dims_range, int dim) {
// 	int rules_num = rules.size();
// 	for (int i = 0; i < rules_num; ++i)
// 		for (int j = 0; j < 5; ++j) {
// 			int k = dim_prefix_len[j] - rules[i]->prefix_len[j];
// 			rules[i]->reduced_key[j] = (uint64_t)rules[i]->key[j] >> k << k;
// 		}
// 	memset(dim_len_rules_num, 0, sizeof(dim_len_rules_num));
// 	for (int i = 0; i < rules_num; ++i) {
// 		// printf("%d\n", rules[i]->prefix_len[dim]);
// 		++dim_len_rules_num[rules[i]->prefix_len[dim]];
// 	}
// 	// for (int i = 0; i <= dim_prefix_len[dim]; ++i)
// 	// 	printf("%d : %d\n", i, dim_len_rules_num[i]);

// 	memset(tuple_cost_sum, 0, sizeof(tuple_cost_sum));
// 	for (int i = dim_prefix_len[dim]; i >= 0; --i)
// 	// for (int i = 15; i >= 15; --i)
// 		CalculateDimRangeX(rules, dims_range, dim, i);
// 	// for (int i = 0; i <= dim_prefix_len[dim]; ++i) {
// 	// 	for (int j = 0; j <= dim_prefix_len[dim]; ++j)
// 	// 		printf("%llu ", tuple_cost_sum[i][j]);
// 	// 	printf("\n");
// 	// }
// 	DynamicProgrammingX(dim);
// 	// printf("GetDpRange\n");
// 	GetDpRange(0, dim_prefix_len[dim], dims_range, dim);
// 	return dp_cost[0][dim_prefix_len[dim]];
// }

// DimsRange GetDimRange(vector<Rule*> &rules, int prefix_dims_num) {
// 	// printf("DimRange %d\n", prefix_dims_num);
// 	int rules_num = rules.size();
//     if (rules_num == 0) {
//     	printf("rules_num = 0\n");
//     	exit(1);
//     }
//     vector<DimRule*> dim_rules = GetDimRules(rules);
//     DimsRange dims_range = InitDimsRange();

//     uint64_t cost = 0, pre_cost = 1ULL << 60;
//     for (int round = 0; round < 100 ; ++round) {
//     	// printf("\n\n\nround %d\n", round);
//     	for (int dim = 0; dim < 5; ++dim) {
//     		// printf("\n dim %d\n", dim);

//     		cost = CalculateDimRange(dim_rules, dims_range, dim);
//     		// for (int i = 0; i < dim_prefix_len[dim]; ++i)
//     		// 	printf("%d ", dims_range.reduced[dim][i]);
//     		// printf("\n");
//     	}
//     	if (cost > pre_cost) {
//     		printf("Wong cost %lu pre_cost %lu\n", cost, pre_cost);
//     		exit(1);
//     	}
//     	if (cost > pre_cost * 0.95) {
//     		break;
//     	}
//     	pre_cost = cost;
//     	// printf("-----pre_cost %llu\n", pre_cost);
//     }
// 	// CalculateDimCost(dim_rules, dims_range, 0);
// 	return dims_range;
// }
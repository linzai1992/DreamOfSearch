#include "server/backend/searcher.h"

#include "common/flags.h"
#include "common/util.h"
#include "server/backend/doc_builder.h"
#include "thirdparty/cJSON.h"

DEFINE_string(supply_data,
              "db_datas/supplies.data",
              "");

#define MakeRequestParams(field)  \
  itrt = params.find(rp->names().field());  \
  if (itrt != params.end()) {  \
    rp->set_##field(StringToInt(itrt->second));  \
    ++count;  \
  }

Searcher::Searcher()
    : supply_indexer_(new Indexer(FLAGS_supply_data)) {
  index_searcher_.reset(
      new InverseDoclistSearcher(supply_indexer_));
}

cJSON* Searcher::SearchSupply(
    const map<string, string>& params,
    cJSON* running_info) const {
  RequestParams rp;
  if (!BuildRequestParams(params, &rp, running_info)) {
    cJSON_AddItemToObject(
        running_info,
        "param error",
        cJSON_CreateString("url is wrong. Please check!"));
    return NULL;
  }
  vector<DocId> ids;

  long long start = ustime();
  index_searcher_->SearchDocId(rp, &ids, running_info);
  long long end = ustime();

  cJSON_AddItemToObject(
      running_info,
      "search cost(us)",
      cJSON_CreateNumber(end - start));

  cJSON* reply = cJSON_CreateObject();
  string ids_str;
  for (size_t i = 0; i < ids.size(); ++i) {
    StringAppendF(
        &ids_str,
        "%d%c",
        supply_indexer_->FromDocIdToRawDocId(ids[i]),
        i == (ids.size() - 1) ? '\0' : ',');
  }
  cJSON_AddItemToObject(
      reply,
      "ids",
      cJSON_CreateString(ids_str.c_str()));
  return reply;
}

bool Searcher::BuildRequestParams(
    const map<string, string>& params,
    RequestParams* rp,
    cJSON* running_info) const {
  map<string, string>::const_iterator itrt;

  rp->set_page_size(20);
  rp->set_page_no(0);

  int count = 0;
  MakeRequestParams(product_id);
  MakeRequestParams(breed_id);
  MakeRequestParams(supply_province_id);
  MakeRequestParams(supply_city_id);
  MakeRequestParams(supply_county_id);
  MakeRequestParams(page_size);
  MakeRequestParams(page_no);

  return count;
}

cJSON* Searcher::AddNewDoc(
    const map<string, string>& params,
    cJSON* running_info) {
  shared_ptr<DocBuilder>& builder =
      supply_indexer_->GetDocBuilder();
  string add_status;
  string line_doc = "";
  int ret = 0;
  supply_indexer_->AddDocToIndex(
      builder->GetRawDocFromString(line_doc));
  cJSON* reply = cJSON_CreateObject();
  string flag = "add doc status";
  if (ret) {
    add_status = "Ok";
  } else {
    add_status = "Fail";
  }
  cJSON_AddItemToObject(
      reply,
      flag.c_str(),
      cJSON_CreateString(add_status.c_str()));
  return reply;
}

void Searcher::BuildIndexFromFile() {
  supply_indexer_->Build();
}

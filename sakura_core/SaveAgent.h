#pragma once

class SaveAgent : public DocListenerEx {
public:
	SaveAgent();
	CallbackResultType OnCheckSave(SaveInfo* pSaveInfo);
	void OnBeforeSave(const SaveInfo& saveInfo);
	void OnSave(const SaveInfo& saveInfo);
	void OnAfterSave(const SaveInfo& saveInfo);
	void OnFinalSave(SaveResultType eSaveResult);
private:
	SaveInfo	saveInfoForRollback;
};


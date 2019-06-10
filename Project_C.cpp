//Project_C
//Project for C++ course

#include<string>
#include<iostream>
#include<sstream>
#include<chrono>
#include<map>

using namespace std;
class ISP;
class Outbox;
class Inbox;
class EmailAccount;
class Drafts;
class BaseFolder;
class Message;
class Name;
template<typename T> class Vec;

template<typename T> class Vec {
public:
   Vec();
   Vec(int n);
   Vec(int n, const T &a);
   Vec(const Vec &orig);
   Vec& operator= (const Vec &rhs);
   ~Vec();
   int capacity() const { return _capacity; }
   int size() const { return _size; } 
   T front() const;
   T back() const;
   void clear();
   void pop_back();
   void push_back(const T &a);
   T& at(int n);
   T& operator[] (int n);
   const T& operator[] (int n) const;
   void erase(int n);

private:
   void allocate();
   void release();
   int _capacity;
   int _size;
   T * _vec;
};

class Name {
public:
   Name() {}
   Name(string s) { set(s); }
   void set(string s) {
      const string _domain = "@qc.cuny.edu";
      istringstream iss(s);
      iss >> _name; 
      if (_name == "") _address = "";
      else{
         _address = _name + _domain;
         for(int i = 0; i < _address.length(); ++i) 
            _address[i] = tolower(_address[i]);
      }
   }

   string name() const { return _name; } 
   string address() const { return _address; } 
   friend bool operator== (Name n1, Name n2){
      if (n1.address() == n2.address()) return true;
      return false;
   }

   friend bool operator< (Name n1, Name n2){
      if(n1.address() < n2.address()) return true;
      return false;
   }

private:
   string _name;
   string _address;
};

class Message {
public:
   Message(string f){
      _from.set(f);
      _date = 0;
   }
   Message(string f, string t){
      _from.set(f);
      _to.set(t);
      _date = 0;
   }
   Message(string f, string t, string s){
      _from.set(f);
      _to.set(t);
      _subject = s;
      _date = 0;
   }
   Message(string f, string t, string s, string txt){
      _from.set(f);
      _to.set(t);
      _subject = s;
      _text = txt;
      _date = 0;      
   }
   Message(const Message &orig){
      _from = orig._from;
      _to = orig._to;
      _subject = orig._subject;
      _text = orig._text;
      _date = orig._date;
   }
   const Name& from() const { return _from; }
   const Name& to() const { return _to; }
   string subject() const { return _subject; }
   string text() const { return _text; }
   string date() const {
      if (_date > 0) {
         tm * timeinfo = std::localtime(&_date); 
         const int len = 256;
         char buffer[len];
         strftime (buffer,len,"%c",timeinfo);
         return buffer;
      }
      else return "";
   }

   void send() { setDate(); }
   void setRecipient(string t) { _to.set(t); }
   void setSubject(string s) { _subject = s; }
   void setText(string txt) { _text = txt; }
   void prependText(string t); 
   void appendText(string t); 
   void print() const {
      cout << "From: " << _from.name() << " <" << _from.address() << ">" << endl;
      cout << "To: " << _to.name() << " <" << _to.address() << ">" << endl;
      cout << "Subject: " << _subject << endl;
      cout << "Date: " << date() << endl;
      cout << _text << endl;
      cout << endl;
   }

private:
   void setDate() {
      auto t_now = chrono::system_clock::now();
      _date = chrono::system_clock::to_time_t(t_now);
   }
   Name _from; 
   Name _to; 
   string _subject;
   string _text;
   time_t _date; 
};

class Drafts {
public:
   Drafts(EmailAccount *ac);
   ~Drafts(){
      map<int, Message*>::iterator m_it; 
      for (m_it = _drafts.begin(); m_it != _drafts.end(); ++m_it)
      {
         delete m_it->second;
         _drafts.erase(m_it);
      }
   }

      
   void display() const;
      
   void send(int n);
      
   void erase(int n){
      delete _drafts[n];
      _drafts.erase(n);
   }
   Message* addDraft();
   Message* addDraft(Message *m){
      int key = newKey();
      _drafts[key] = m;
      return m;
   }
   Message* getDraft(int n) { return _drafts[n]; }
   Message* operator[](int n) { return _drafts[n]; }

private:
   int newKey(){ return ++_newKey; } // _newKey cannot be declared as static class data because not every email account shares the same _newKey.
   Drafts(const Drafts &orig) = delete;
   Drafts& operator=(const Drafts &rhs) = delete;
   int _newKey;
   map<int, Message*> _drafts;
   EmailAccount *_ac;
};

class EmailAccount {
public:
   EmailAccount(string s);
   ~EmailAccount(){ }
   const Name& owner() const { return _owner; }
   Drafts& drafts() { return *_drafts; }
   BaseFolder* in() { return _in; } 
   BaseFolder* out() { return _out; } 
   void send(Message *m);
   void receive(Message *m);
   void insert(Message *m);
   
   
private:
   EmailAccount(const EmailAccount &orig) = delete;
   EmailAccount& operator=(const EmailAccount &rhs) = delete;
   Name _owner;
   Drafts * _drafts;
   BaseFolder * _in;
   BaseFolder * _out;
};


class BaseFolder {
public:
   int size() const { return _msg.size(); }
   void display() const{
      cout << _ac->owner().name() << " " << type() << endl;
      if (_msg.size() <= 1) cout << "no messages to display" << endl;
      else{
         for (int i = 1; i < _msg.size(); ++i){
            cout << i << endl;
            const Name &tmp = tofrom(_msg[i]);
            cout << tmp.name();
            cout << _msg[i]->subject();
         }
      }
   }
   void erase(int n){
      if ((n < 1) || (n >= _msg.size())) return;
      else{
         delete _msg[n];
         _msg.erase(n);
      }
   }
   void forward(int n) const{
      if ((n < 1) || (n >= _msg.size())) return;
      else{
         const Message *m = _msg[n];
         string fwd_subject = "Fwd:  " + m->subject();
         Message *ptr = new Message(_ac->owner().name(), "", fwd_subject, m->text());
         _ac->insert( ptr );
  
      }
   }

   void print(int n) const{
      
      if ((n < 1) || (n >= _msg.size())) return;
      else _msg[n]->print();
   }
   void receive(const Message *m) { _msg.push_back(m); }
   void reply(int n) const{
      if ((n < 1) || (n >= _msg.size())) return;
      else{
         const Message *m = _msg[n];
         string subject = "Re:  " + m->subject();
         const Name& tmp = tofrom(m);
         Message *ptr = new Message(_ac->owner().name(), tmp.name(), subject, m->text());
         _ac->insert( ptr ); 
      }
   }
   virtual ~BaseFolder(){
      for (int i = 0; i < _msg.size(); ++i)
         delete _msg[i];
   }
   
protected:
   BaseFolder(EmailAccount *ac)  { _ac=ac;_msg.push_back(NULL); }
   virtual string type() const =0; 
   virtual const Name& tofrom(const Message *m) const=0; 
   Vec<const Message*> _msg;
   EmailAccount *_ac;
};


class Inbox : public BaseFolder {
public:
   Inbox(EmailAccount *ac) : BaseFolder(ac){
   }
   ~Inbox(){
      for (int i = 0; i < _msg.size(); ++i)
         delete _msg[i];
   }

protected:
   string type() const { return "inbox"; }
   const Name& tofrom(const Message *m) const { return m->from(); }
};


class Outbox : public BaseFolder {
public:
   Outbox(EmailAccount *ac) : BaseFolder(ac){
   }
   ~Outbox(){
      for (int i = 0; i < _msg.size(); ++i)
         delete _msg[i];
   }

protected:
   string type() const { return "outbox"; }
   const Name& tofrom(const Message *m) const { return m->to(); }
};

class ISP {
public:
   static void addAccount(EmailAccount *e) {
   _accounts[e->owner()] = e;
   }
   static void send(Message *m){
      EmailAccount *ac = _accounts[m->to()];
      if (ac == NULL){
         cout << "Delivery failed, to recipient: " << m->to().name() << endl;
         return;
      }
      else{
         Message *clone = new Message(*m);
         ac->receive(clone);
         return;
      }
   }
   
private:
   ISP() {}
   static map<Name, EmailAccount*> _accounts;
};
map<Name, EmailAccount*> ISP::_accounts;
//Drafts
Drafts::Drafts(EmailAccount *ac){
      _ac = ac;
      _newKey = 0;
      _drafts[0] = NULL;
   }

void Drafts::display()const {
      cout << _ac->owner().name() << " drafts: " << endl;
      if (_drafts.size() <= 1) cout << "no messages to display" << endl;
      else{
          
         for (map<int, Message*>::const_iterator mit = _drafts.begin(); mit != _drafts.end(); mit++)
         {
            if (mit->first == 0) continue;
            cout << mit->first<<endl;
 	    const Message *ptr = mit->second; 
            cout <<"to : "<<ptr->to().name()<<endl;
            cout <<"subject : "<<ptr->subject()<<endl;
         }
      }
    } 
    
void Drafts::send(int n){
      Message *ptr = _drafts[n];
      if (ptr == NULL){
         cout << "Message not found";
         return;
      }
      else if (ptr->to().address() == "") {
         cout << "You must specify a recipient" << endl;
         return;
      }
      else{
         ptr->send();
         _drafts.erase(n);
         _ac->send( ptr );

      }
   }
   
Message* Drafts::addDraft(){
      int key = newKey();
      Message *ptr = new Message(_ac->owner().name());
      _drafts[key] = ptr;
      return ptr;
   }

//EmailAccount  
EmailAccount::EmailAccount(string s){
      _owner.set(s);
      _drafts = new Drafts(this);
      _in = new Inbox(this);
      _out = new Outbox(this);
      ISP::addAccount(this);
   }
   
void EmailAccount::send(Message *m) {
      _out->receive(m);
      ISP::send(m);
   }
   
void EmailAccount::receive(Message *m) { _in->receive(m); }

void EmailAccount::insert(Message *m) { _drafts->addDraft(m); }

//Vec
template<typename T>
Vec<T>::Vec() : _capacity(0), _size(0), _vec(NULL) {}

template<typename T>
void Vec<T>::release(){
   if (_vec != NULL) delete [] _vec;
   _vec = NULL;
}

template<typename T>
void Vec<T>::allocate(){
   if (_capacity > 0) _vec = new T[_capacity];
   else _vec = NULL;
}

template<typename T>
Vec<T>& Vec<T>::operator= (const Vec<T> &rhs){
   if (this == &rhs) return *this;
   else{
      _capacity = rhs._size;
      _size = rhs._size;
      release();
      allocate();
      for (int i = 0; i < _size; ++i)
         _vec[i] = rhs._vec[i];
      return *this;
   }
}


template<typename T>
Vec<T>::Vec(int n){
   if (n <= 0){
      _capacity = 0;
      _size = 0;
      _vec = NULL;
   }
   else{
      _capacity = n;
      _size = n;
      allocate();
   }
}

template<typename T>
Vec<T>::Vec(int n, const T &a){
   if (n <= 0){
      _capacity = 0;
      _size = 0;
      _vec = NULL;
   }
   else{
      _capacity = n;
      _size = n;
      allocate();
      for (int i = 0; i < _capacity; ++i)
	 _vec[i] = a;
   }
}

template<typename T>
Vec<T>::~Vec() { release(); }

template<typename T>
T Vec<T>::front() const {
	 if (_size > 0) return _vec[0];
	 else {
	    T default_obj;
	    return default_obj;
	 }
}

template<typename T>
T Vec<T>::back() const {
	 if (_size > 0) return _vec[_size - 1];
	 else {
	    T default_obj;
	    return default_obj;
	 }
      }

template<typename T>
Vec<T>::Vec(const Vec &orig){
	 _capacity = orig._size;
	 _size = orig._size;
	 allocate();
	 for (int i = 0; i < _size; ++i)
	    _vec[i] = orig._vec[i];
      }

template<typename T>
void Vec<T>::clear() { _size = 0;}

template<typename T>
void Vec<T>::pop_back() { if (_size > 0) _size -= 1;}

template<typename T>
void Vec<T>::push_back(const T &a){
	 if (_capacity > _size){
	    _vec[_size] = a;
	    _size++;
	 }
	 else if (_capacity == 0) _capacity = 1;
         else{
	    _capacity *= 2;
	    T *oldvec = _vec;
	    allocate();
	    if (oldvec != NULL){
	       for (int i = 0; i < _size; i++)
                  _vec[i] = oldvec[i];
	    delete [] oldvec;
	    }
	    _vec[_size] = a;
	    _size++;
	 }
      }

template<typename T>
T& Vec<T>::at(int n){
	 if ((n >= 0) && (n < _size)) return _vec[n];
	 else{
	    T *pnull = NULL;
	    return *pnull;
	 }
      }

template<typename T>
T& Vec<T>::operator[] (int n) { return at(n);}

template<typename T>
const T& Vec<T>::operator[] (int n) const {
	 if ((n >= 0) && (n < _size)) return _vec[n];
	 else{
	    T *pnull = NULL;
	    return *pnull;
	 }
      } 

template<typename T>
void Vec<T>::erase(int n){
   if ((n < 0) || (n >= _size)) return;
   for (int i = n; i < _size - 1; ++i)
      _vec[i] = _vec[i + 1];
   _size = _size - 1;
}


int main(){
	
	return 0;
}
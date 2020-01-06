#pragma once
namespace XDataStruct {
		template<class type>
		class XVector
		{
		public:
			XVector();
			~XVector();
			void Doublesize();
			type& get(int i);
			void add(type d);
			
		private:
			type* datas;
			int size;
			int index;

		};
		template<class type>
		inline XVector<type>::XVector()
		{
			size = 10;
			datas = new type[size];

		}
		template<class type>
		inline XVector<type>::~XVector()
		{
			if (datas != nullptr)
			{
				delete[] datas;
			}

		}
		template<class type>
		inline void XVector<type>::Doublesize()
		{
			type* temp = datas;
			int temps = size;
			size *= 2;
			datas = new type[size];
			for (int i = 0; i < temps; i++)
			{
				datas[i] = temp[i];
			}
			delete[] temp;

		}
		template<class type>
		inline type& XVector<type>::get(int i)
		{

			if (i < size)
			{
				return datas[i];
			}
			
		}
		template<class type>
		inline void XVector<type>::add(type d)
		{
			
			if(index<size)
				Doublesize();
			datas[++index] = d;
		}
}
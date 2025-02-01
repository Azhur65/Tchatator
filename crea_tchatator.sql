drop schema if exists chatator cascade;
create schema chatator;

set schema 'chatator';

create table client(
	client_id varchar primary key,
	api_key varchar,
	status varchar,
	banni boolean
);

create table conversation(
	client_id_1 varchar,
	client_id_2 varchar,
	bloque boolean,
	date_deblocage timestamp,
	constraint conversation_pk primary key (client_id_1,client_id_2),
	constraint conversation_fk_1 foreign key (client_id_1) references client(client_id),
	constraint conversation_fk_2 foreign key (client_id_2) references client(client_id)
);

create table message(
	message_id serial primary key,
	message varchar(1000),
	envoyeur varchar,
	receveur varchar,
	date timestamp,
	modifie boolean,
	date_modification date,
	constraint message_fk_1 foreign key (envoyeur) references client(client_id),
	constraint message_fk_2 foreign key (receveur) references client(client_id)

);

insert into chatator.client(client_id, api_key, status, banni) 
	values('Co-0001', 'KzGF7', 'administrateur', false),
	('Co-0002', 'wxBmr', 'professionnel', false),
	('Co-0003', 'IKpAt', 'client', false);

insert into chatator.conversation(client_id_1, client_id_2, bloque, date_deblocage)
	values('Co-0001', 'Co-0002', false, NULL),
	('Co-0001', 'Co-0003', false, NULL),
	('Co-0003', 'Co-0002', false, NULL);
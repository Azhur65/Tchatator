drop schema if exists chatator cascade;
create schema chatator;

set schema 'chatator';

create table client(
	client_id serial primary key,
	api_key varchar,
	status varchar
);

create table conversation(
	client_id_1 int,
	client_id_2 int,
	bloque boolean,
	date_deblocage timestamp,
	constraint conversation_pk primary key (client_id_1,client_id_2),
	constraint conversation_fk_1 foreign key (client_id_1) references client(client_id),
	constraint conversation_fk_2 foreign key (client_id_2) references client(client_id)
);

create table message(
	message_id serial primary key,
	message varchar(1000),
	envoyeur int,
	receveur int,
	date timestamp,
	modifie boolean,
	date_modification date,
	constraint message_fk_1 foreign key (envoyeur) references client(client_id),
	constraint message_fk_2 foreign key (receveur) references client(client_id)

);
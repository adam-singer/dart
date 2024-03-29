// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#include "vm/bigint_operations.h"
#include "vm/object.h"
#include "vm/object_store.h"
#include "vm/snapshot.h"
#include "vm/visitor.h"

namespace dart {

#define NEW_OBJECT(type)                                                       \
  ((kind == Snapshot::kFull) ? reader->New##type() : type::New())

#define NEW_OBJECT_WITH_LEN(type, len)                                         \
  ((kind == Snapshot::kFull) ? reader->New##type(len) : type::New(len))


static uword ZoneAllocator(intptr_t size) {
  Zone* zone = Isolate::Current()->current_zone();
  return zone->Allocate(size);
}


RawClass* Class::ReadFrom(SnapshotReader* reader,
                          intptr_t object_id,
                          intptr_t tags,
                          Snapshot::Kind kind) {
  ASSERT(reader != NULL);

  Class& cls = Class::ZoneHandle(reader->isolate(), Class::null());
  if ((kind == Snapshot::kFull) ||
      (kind == Snapshot::kScript && !RawObject::IsCreatedFromSnapshot(tags))) {
    // Read in the base information.
    ObjectKind object_kind = reader->Read<ObjectKind>();

    // Allocate class object of specified kind.
    if (kind == Snapshot::kFull) {
      cls = reader->NewClass(object_kind);
    } else {
      cls = Class::GetClass(object_kind);
    }
    reader->AddBackwardReference(object_id, &cls);

    // Set the object tags.
    cls.set_tags(tags);

    // Set all non object fields.
    cls.set_instance_size(reader->ReadIntptrValue());
    cls.set_type_arguments_instance_field_offset(reader->ReadIntptrValue());
    cls.set_next_field_offset(reader->ReadIntptrValue());
    cls.set_num_native_fields(reader->ReadIntptrValue());
    cls.set_token_index(reader->ReadIntptrValue());
    cls.set_class_state(reader->Read<int8_t>());
    if (reader->Read<bool>()) {
      cls.set_is_const();
    }
    if (reader->Read<bool>()) {
      cls.set_is_interface();
    }

    // Set all the object fields.
    // TODO(5411462): Need to assert No GC can happen here, even though
    // allocations may happen.
    intptr_t num_flds = (cls.raw()->to() - cls.raw()->from());
    for (intptr_t i = 0; i <= num_flds; i++) {
      *(cls.raw()->from() + i) = reader->ReadObject();
    }
  } else {
    cls ^= reader->ReadClassId(object_id);
  }
  return cls.raw();
}


void RawClass::WriteTo(SnapshotWriter* writer,
                       intptr_t object_id,
                       Snapshot::Kind kind) {
  ASSERT(writer != NULL);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  if ((kind == Snapshot::kFull) ||
      (kind == Snapshot::kScript && !IsCreatedFromSnapshot())) {
    // Write out the class and tags information.
    writer->WriteObjectHeader(Object::kClassClass, ptr()->tags_);

    // Write out all the non object pointer fields.
    // NOTE: cpp_vtable_ is not written.
    writer->Write<ObjectKind>(ptr()->instance_kind_);
    writer->WriteIntptrValue(ptr()->instance_size_);
    writer->WriteIntptrValue(ptr()->type_arguments_instance_field_offset_);
    writer->WriteIntptrValue(ptr()->next_field_offset_);
    writer->WriteIntptrValue(ptr()->num_native_fields_);
    writer->WriteIntptrValue(ptr()->token_index_);
    writer->Write<int8_t>(ptr()->class_state_);
    writer->Write<bool>(ptr()->is_const_);
    writer->Write<bool>(ptr()->is_interface_);

    // Write out all the object pointer fields.
    SnapshotWriterVisitor visitor(writer);
    visitor.VisitPointers(from(), to());
  } else {
    writer->WriteClassId(this);
  }
}


RawUnresolvedClass* UnresolvedClass::ReadFrom(SnapshotReader* reader,
                                              intptr_t object_id,
                                              intptr_t tags,
                                              Snapshot::Kind kind) {
  ASSERT(reader != NULL);

  // Allocate parameterized type object.
  UnresolvedClass& unresolved_class = UnresolvedClass::ZoneHandle(
      reader->isolate(), NEW_OBJECT(UnresolvedClass));
  reader->AddBackwardReference(object_id, &unresolved_class);

  // Set the object tags.
  unresolved_class.set_tags(tags);

  // Set all non object fields.
  unresolved_class.set_token_index(reader->ReadIntptrValue());

  // Set all the object fields.
  // TODO(5411462): Need to assert No GC can happen here, even though
  // allocations may happen.
  intptr_t num_flds = (unresolved_class.raw()->to() -
                       unresolved_class.raw()->from());
  for (intptr_t i = 0; i <= num_flds; i++) {
    *(unresolved_class.raw()->from() + i) = reader->ReadObject();
  }
  return unresolved_class.raw();
}


void RawUnresolvedClass::WriteTo(SnapshotWriter* writer,
                                 intptr_t object_id,
                                 Snapshot::Kind kind) {
  ASSERT(writer != NULL);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kUnresolvedClassClass, ptr()->tags_);

  // Write out all the non object pointer fields.
  writer->WriteIntptrValue(ptr()->token_index_);

  // Write out all the object pointer fields.
  SnapshotWriterVisitor visitor(writer);
  visitor.VisitPointers(from(), to());
}


RawAbstractType* AbstractType::ReadFrom(SnapshotReader* reader,
                                        intptr_t object_id,
                                        intptr_t tags,
                                        Snapshot::Kind kind) {
  UNREACHABLE();  // AbstractType is an abstract class.
  return NULL;
}


void RawAbstractType::WriteTo(SnapshotWriter* writer,
                              intptr_t object_id,
                              Snapshot::Kind kind) {
  UNREACHABLE();  // AbstractType is an abstract class.
}


RawType* Type::ReadFrom(SnapshotReader* reader,
                        intptr_t object_id,
                        intptr_t tags,
                        Snapshot::Kind kind) {
  ASSERT(reader != NULL);

  // Allocate parameterized type object.
  Type& parameterized_type = Type::ZoneHandle(
      reader->isolate(), NEW_OBJECT(Type));
  reader->AddBackwardReference(object_id, &parameterized_type);

  // Set the object tags.
  parameterized_type.set_tags(tags);

  // Set all non object fields.
  parameterized_type.set_token_index(reader->ReadIntptrValue());
  parameterized_type.set_type_state(reader->Read<int8_t>());

  // Set all the object fields.
  // TODO(5411462): Need to assert No GC can happen here, even though
  // allocations may happen.
  intptr_t num_flds = (parameterized_type.raw()->to() -
                       parameterized_type.raw()->from());
  for (intptr_t i = 0; i <= num_flds; i++) {
    *(parameterized_type.raw()->from() + i) = reader->ReadObject();
  }

  // If object needs to be a canonical object, Canonicalize it.
  if ((kind != Snapshot::kFull) && parameterized_type.IsCanonical()) {
    parameterized_type ^= parameterized_type.Canonicalize();
  }
  return parameterized_type.raw();
}


void RawType::WriteTo(SnapshotWriter* writer,
                      intptr_t object_id,
                      Snapshot::Kind kind) {
  ASSERT(writer != NULL);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kTypeClass, ptr()->tags_);

  // Write out all the non object pointer fields.
  writer->WriteIntptrValue(ptr()->token_index_);
  writer->Write<int8_t>(ptr()->type_state_);

  // Write out all the object pointer fields.
  SnapshotWriterVisitor visitor(writer);
  visitor.VisitPointers(from(), to());
}


RawTypeParameter* TypeParameter::ReadFrom(SnapshotReader* reader,
                                          intptr_t object_id,
                                          intptr_t tags,
                                          Snapshot::Kind kind) {
  ASSERT(reader != NULL);

  // Allocate type parameter object.
  TypeParameter& type_parameter = TypeParameter::ZoneHandle(
          reader->isolate(), NEW_OBJECT(TypeParameter));
  reader->AddBackwardReference(object_id, &type_parameter);

  // Set the object tags.
  type_parameter.set_tags(tags);

  // Set all non object fields.
  type_parameter.set_index(reader->ReadIntptrValue());
  type_parameter.set_token_index(reader->ReadIntptrValue());
  type_parameter.set_type_state(reader->Read<int8_t>());

  // Set all the object fields.
  // TODO(5411462): Need to assert No GC can happen here, even though
  // allocations may happen.
  intptr_t num_flds = (type_parameter.raw()->to() -
                       type_parameter.raw()->from());
  for (intptr_t i = 0; i <= num_flds; i++) {
    *(type_parameter.raw()->from() + i) = reader->ReadObject();
  }

  return type_parameter.raw();
}


void RawTypeParameter::WriteTo(SnapshotWriter* writer,
                               intptr_t object_id,
                               Snapshot::Kind kind) {
  ASSERT(writer != NULL);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kTypeParameterClass, ptr()->tags_);

  // Write out all the non object pointer fields.
  writer->WriteIntptrValue(ptr()->index_);
  writer->WriteIntptrValue(ptr()->token_index_);
  writer->Write<int8_t>(ptr()->type_state_);

  // Write out all the object pointer fields.
  SnapshotWriterVisitor visitor(writer);
  visitor.VisitPointers(from(), to());
}


RawInstantiatedType* InstantiatedType::ReadFrom(SnapshotReader* reader,
                                                intptr_t object_id,
                                                intptr_t tags,
                                                Snapshot::Kind kind) {
  ASSERT(reader != NULL);
  ASSERT(kind == Snapshot::kMessage);

  // Allocate instantiated type object.
  InstantiatedType& instantiated_type =
      InstantiatedType::ZoneHandle(reader->isolate(), InstantiatedType::New());
  reader->AddBackwardReference(object_id, &instantiated_type);

  // Set the object tags.
  instantiated_type.set_tags(tags);

  // Now set all the object fields.
  // TODO(5411462): Need to assert No GC can happen here, even though
  // allocations may happen.
  intptr_t num_flds = (instantiated_type.raw()->to() -
                       instantiated_type.raw()->from());
  for (intptr_t i = 0; i <= num_flds; i++) {
    *(instantiated_type.raw()->from() + i) = reader->ReadObject();
  }
  return instantiated_type.raw();
}


void RawInstantiatedType::WriteTo(SnapshotWriter* writer,
                                  intptr_t object_id,
                                  Snapshot::Kind kind) {
  ASSERT(writer != NULL);
  ASSERT(kind == Snapshot::kMessage);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kInstantiatedTypeClass, ptr()->tags_);

  // Write out all the object pointer fields.
  SnapshotWriterVisitor visitor(writer);
  visitor.VisitPointers(from(), to());
}


RawAbstractTypeArguments* AbstractTypeArguments::ReadFrom(
    SnapshotReader* reader,
    intptr_t object_id,
    intptr_t tags,
    Snapshot::Kind kind) {
  UNREACHABLE();  // AbstractTypeArguments is an abstract class.
  return TypeArguments::null();
}


void RawAbstractTypeArguments::WriteTo(SnapshotWriter* writer,
                                       intptr_t object_id,
                                       Snapshot::Kind kind) {
  UNREACHABLE();  // AbstractTypeArguments is an abstract class.
}


RawTypeArguments* TypeArguments::ReadFrom(SnapshotReader* reader,
                                          intptr_t object_id,
                                          intptr_t tags,
                                          Snapshot::Kind kind) {
  ASSERT(reader != NULL);

  // Read the length so that we can determine instance size to allocate.
  intptr_t len = reader->ReadSmiValue();

  TypeArguments& type_arguments = TypeArguments::ZoneHandle(
      reader->isolate(), NEW_OBJECT_WITH_LEN(TypeArguments, len));
  reader->AddBackwardReference(object_id, &type_arguments);

  // Now set all the object fields.
  for (intptr_t i = 0; i < len; i++) {
    *reader->TypeHandle() ^= reader->ReadObject();
    type_arguments.SetTypeAt(i, *reader->TypeHandle());
  }

  // Set the object tags (This is done after setting the object fields
  // because 'SetTypeAt' has an assertion to check if the object is not
  // already canonical).
  type_arguments.set_tags(tags);

  // If object needs to be a canonical object, Canonicalize it.
  if ((kind != Snapshot::kFull) && type_arguments.IsCanonical()) {
    type_arguments ^= type_arguments.Canonicalize();
  }
  return type_arguments.raw();
}


void RawTypeArguments::WriteTo(SnapshotWriter* writer,
                               intptr_t object_id,
                               Snapshot::Kind kind) {
  ASSERT(writer != NULL);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kTypeArgumentsClass, ptr()->tags_);

  // Write out the length field.
  writer->Write<RawObject*>(ptr()->length_);

  // Write out the individual types.
  intptr_t len = Smi::Value(ptr()->length_);
  for (intptr_t i = 0; i < len; i++) {
    writer->WriteObject(ptr()->types_[i]);
  }
}


RawInstantiatedTypeArguments* InstantiatedTypeArguments::ReadFrom(
    SnapshotReader* reader,
    intptr_t object_id,
    intptr_t tags,
    Snapshot::Kind kind) {
  ASSERT(reader != NULL);
  ASSERT(kind == Snapshot::kMessage);

  // Allocate instantiated types object.
  InstantiatedTypeArguments& instantiated_type_arguments =
      InstantiatedTypeArguments::ZoneHandle(reader->isolate(),
                                            InstantiatedTypeArguments::New());
  reader->AddBackwardReference(object_id, &instantiated_type_arguments);

  // Set the object tags.
  instantiated_type_arguments.set_tags(tags);

  // Set all the object fields.
  // TODO(5411462): Need to assert No GC can happen here, even though
  // allocations may happen.
  intptr_t num_flds = (instantiated_type_arguments.raw()->to() -
                       instantiated_type_arguments.raw()->from());
  for (intptr_t i = 0; i <= num_flds; i++) {
    *(instantiated_type_arguments.raw()->from() + i) = reader->ReadObject();
  }
  return instantiated_type_arguments.raw();
}


void RawInstantiatedTypeArguments::WriteTo(SnapshotWriter* writer,
                                           intptr_t object_id,
                                           Snapshot::Kind kind) {
  ASSERT(writer != NULL);
  ASSERT(kind == Snapshot::kMessage);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kInstantiatedTypeArgumentsClass,
                            ptr()->tags_);

  // Write out all the object pointer fields.
  SnapshotWriterVisitor visitor(writer);
  visitor.VisitPointers(from(), to());
}


RawFunction* Function::ReadFrom(SnapshotReader* reader,
                                intptr_t object_id,
                                intptr_t tags,
                                Snapshot::Kind kind) {
  ASSERT(reader != NULL);
  ASSERT(kind != Snapshot::kMessage && !RawObject::IsCreatedFromSnapshot(tags));

  // Allocate function object.
  Function& func = Function::ZoneHandle(
      reader->isolate(), NEW_OBJECT(Function));
  reader->AddBackwardReference(object_id, &func);

  // Set the object tags.
  func.set_tags(tags);

  // Set all the non object fields.
  func.set_token_index(reader->ReadIntptrValue());
  func.set_end_token_index(reader->ReadIntptrValue());
  func.set_num_fixed_parameters(reader->ReadIntptrValue());
  func.set_num_optional_parameters(reader->ReadIntptrValue());
  func.set_usage_counter(reader->ReadIntptrValue());
  func.set_deoptimization_counter(reader->ReadIntptrValue());
  func.set_kind(static_cast<RawFunction::Kind >(reader->ReadIntptrValue()));
  func.set_is_static(reader->Read<bool>());
  func.set_is_const(reader->Read<bool>());
  func.set_is_optimizable(reader->Read<bool>());

  // Set all the object fields.
  // TODO(5411462): Need to assert No GC can happen here, even though
  // allocations may happen.
  intptr_t num_flds = (func.raw()->to() - func.raw()->from());
  for (intptr_t i = 0; i <= num_flds; i++) {
    *(func.raw()->from() + i) = reader->ReadObject();
  }

  return func.raw();
}


void RawFunction::WriteTo(SnapshotWriter* writer,
                          intptr_t object_id,
                          Snapshot::Kind kind) {
  ASSERT(writer != NULL);
  ASSERT(kind != Snapshot::kMessage && !IsCreatedFromSnapshot());

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kFunctionClass, ptr()->tags_);

  // Write out all the non object fields.
  writer->WriteIntptrValue(ptr()->token_index_);
  writer->WriteIntptrValue(ptr()->end_token_index_);
  writer->WriteIntptrValue(ptr()->num_fixed_parameters_);
  writer->WriteIntptrValue(ptr()->num_optional_parameters_);
  writer->WriteIntptrValue(ptr()->usage_counter_);
  writer->WriteIntptrValue(ptr()->deoptimization_counter_);
  writer->WriteIntptrValue(ptr()->kind_);
  writer->Write<bool>(ptr()->is_static_);
  writer->Write<bool>(ptr()->is_const_);
  writer->Write<bool>(ptr()->is_optimizable_);

  // Write out all the object pointer fields.
  SnapshotWriterVisitor visitor(writer);
  visitor.VisitPointers(from(), to());
}


RawField* Field::ReadFrom(SnapshotReader* reader,
                          intptr_t object_id,
                          intptr_t tags,
                          Snapshot::Kind kind) {
  ASSERT(reader != NULL);
  ASSERT(kind != Snapshot::kMessage && !RawObject::IsCreatedFromSnapshot(tags));

  // Allocate field object.
  Field& field = Field::ZoneHandle(reader->isolate(), NEW_OBJECT(Field));
  reader->AddBackwardReference(object_id, &field);

  // Set the object tags.
  field.set_tags(tags);

  // Set all non object fields.
  field.set_token_index(reader->ReadIntptrValue());
  field.set_is_static(reader->Read<bool>());
  field.set_is_final(reader->Read<bool>());
  field.set_has_initializer(reader->Read<bool>());

  // Set all the object fields.
  // TODO(5411462): Need to assert No GC can happen here, even though
  // allocations may happen.
  intptr_t num_flds = (field.raw()->to() - field.raw()->from());
  for (intptr_t i = 0; i <= num_flds; i++) {
    *(field.raw()->from() + i) = reader->ReadObject();
  }

  return field.raw();
}


void RawField::WriteTo(SnapshotWriter* writer,
                       intptr_t object_id,
                       Snapshot::Kind kind) {
  ASSERT(writer != NULL);
  ASSERT(kind != Snapshot::kMessage && !IsCreatedFromSnapshot());

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kFieldClass, ptr()->tags_);

  // Write out all the non object fields.
  writer->WriteIntptrValue(ptr()->token_index_);
  writer->Write<bool>(ptr()->is_static_);
  writer->Write<bool>(ptr()->is_final_);
  writer->Write<bool>(ptr()->has_initializer_);

  // Write out all the object pointer fields.
  SnapshotWriterVisitor visitor(writer);
  visitor.VisitPointers(from(), to());
}


RawLiteralToken* LiteralToken::ReadFrom(SnapshotReader* reader,
                                        intptr_t object_id,
                                        intptr_t tags,
                                        Snapshot::Kind kind) {
  ASSERT(reader != NULL);
  ASSERT(kind != Snapshot::kMessage);

  // Create the literal token object.
  LiteralToken& literal_token = LiteralToken::ZoneHandle(
      reader->isolate(), NEW_OBJECT(LiteralToken));
  reader->AddBackwardReference(object_id, &literal_token);

  // Set the object tags.
  literal_token.set_tags(tags);

  // Read the token attributes.
  Token::Kind token_kind = static_cast<Token::Kind>(reader->ReadIntptrValue());
  literal_token.set_kind(token_kind);
  *reader->StringHandle() ^= reader->ReadObject();
  literal_token.set_literal(*reader->StringHandle());
  *reader->ObjectHandle() = reader->ReadObject();
  literal_token.set_value(*reader->ObjectHandle());

  return literal_token.raw();
}


void RawLiteralToken::WriteTo(SnapshotWriter* writer,
                              intptr_t object_id,
                              Snapshot::Kind kind) {
  ASSERT(writer != NULL);
  ASSERT(kind != Snapshot::kMessage);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kLiteralTokenClass, ptr()->tags_);

  // Write out the kind field.
  writer->Write<intptr_t>(ptr()->kind_);

  // Write out literal and value fields.
  writer->WriteObject(ptr()->literal_);
  writer->WriteObject(ptr()->value_);
}


RawTokenStream* TokenStream::ReadFrom(SnapshotReader* reader,
                                      intptr_t object_id,
                                      intptr_t tags,
                                      Snapshot::Kind kind) {
  ASSERT(reader != NULL);
  ASSERT(kind != Snapshot::kMessage && !RawObject::IsCreatedFromSnapshot(tags));

  // Read the length so that we can determine number of tokens to read.
  intptr_t len = reader->ReadSmiValue();

  // Create the token stream object.
  TokenStream& token_stream = TokenStream::ZoneHandle(
      reader->isolate(), NEW_OBJECT_WITH_LEN(TokenStream, len));
  reader->AddBackwardReference(object_id, &token_stream);

  // Set the object tags.
  token_stream.set_tags(tags);

  // Read the token stream into the TokenStream.
  for (intptr_t i = 0; i < len; i++) {
    *reader->ObjectHandle() = reader->ReadObject();
    token_stream.SetTokenAt(i, *reader->ObjectHandle());
  }
  return token_stream.raw();
}


void RawTokenStream::WriteTo(SnapshotWriter* writer,
                             intptr_t object_id,
                             Snapshot::Kind kind) {
  ASSERT(writer != NULL);
  ASSERT(kind != Snapshot::kMessage && !IsCreatedFromSnapshot());

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kTokenStreamClass, ptr()->tags_);

  // Write out the length field.
  writer->Write<RawObject*>(ptr()->length_);

  // Write out the token stream (token kind and literal).
  intptr_t len = Smi::Value(ptr()->length_);
  for (intptr_t i = 0; i < TokenStream::StreamLength(len); i++) {
    writer->WriteObject(ptr()->data_[i]);
  }
}


RawScript* Script::ReadFrom(SnapshotReader* reader,
                            intptr_t object_id,
                            intptr_t tags,
                            Snapshot::Kind kind) {
  ASSERT(reader != NULL);
  ASSERT(kind != Snapshot::kMessage && !RawObject::IsCreatedFromSnapshot(tags));

  // Allocate script object.
  Script& script = Script::ZoneHandle(reader->isolate(), NEW_OBJECT(Script));
  reader->AddBackwardReference(object_id, &script);

  // Set the object tags.
  script.set_tags(tags);

  // Set all the object fields.
  // TODO(5411462): Need to assert No GC can happen here, even though
  // allocations may happen.
  intptr_t num_flds = (script.raw()->to() - script.raw()->from());
  for (intptr_t i = 0; i <= num_flds; i++) {
    *(script.raw()->from() + i) = reader->ReadObject();
  }

  return script.raw();
}


void RawScript::WriteTo(SnapshotWriter* writer,
                        intptr_t object_id,
                        Snapshot::Kind kind) {
  ASSERT(writer != NULL);
  ASSERT(tokens_ != TokenStream::null());
  ASSERT(kind != Snapshot::kMessage && !IsCreatedFromSnapshot());

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kScriptClass, ptr()->tags_);

  // Write out all the object pointer fields.
  SnapshotWriterVisitor visitor(writer);
  visitor.VisitPointers(from(), to());
}


RawLibrary* Library::ReadFrom(SnapshotReader* reader,
                              intptr_t object_id,
                              intptr_t tags,
                              Snapshot::Kind kind) {
  ASSERT(reader != NULL);
  ASSERT(kind != Snapshot::kMessage);

  Library& library = Library::ZoneHandle(reader->isolate(), Library::null());
  reader->AddBackwardReference(object_id, &library);

  if (RawObject::IsCreatedFromSnapshot(tags)) {
    ASSERT(kind != Snapshot::kFull);
    // Lookup the object as it should already exist in the heap.
    *reader->StringHandle() ^= reader->ReadObject();
    library = Library::LookupLibrary(*reader->StringHandle());
  } else {
    // Allocate library object.
    library = NEW_OBJECT(Library);

    // Set the object tags.
    library.set_tags(tags);

    // Set all non object fields.
    library.raw_ptr()->num_imports_ = reader->ReadIntptrValue();
    library.raw_ptr()->num_imported_into_ = reader->ReadIntptrValue();
    library.raw_ptr()->num_anonymous_ = reader->ReadIntptrValue();
    library.raw_ptr()->corelib_imported_ = reader->Read<bool>();
    library.raw_ptr()->load_state_ = reader->Read<int8_t>();
    // The native resolver is not serialized.
    Dart_NativeEntryResolver resolver =
        reader->Read<Dart_NativeEntryResolver>();
    ASSERT(resolver == NULL);
    library.set_native_entry_resolver(resolver);
    // The cache of loaded scripts is not serialized.
    library.raw_ptr()->loaded_scripts_ = Array::null();

    // Set all the object fields.
    // TODO(5411462): Need to assert No GC can happen here, even though
    // allocations may happen.
    intptr_t num_flds = (library.raw()->to() - library.raw()->from());
    for (intptr_t i = 0; i <= num_flds; i++) {
      *(library.raw()->from() + i) = reader->ReadObject();
    }
    if (kind != Snapshot::kFull) {
      library.Register();
    }
  }
  return library.raw();
}


void RawLibrary::WriteTo(SnapshotWriter* writer,
                         intptr_t object_id,
                         Snapshot::Kind kind) {
  ASSERT(writer != NULL);
  ASSERT(kind != Snapshot::kMessage);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kLibraryClass, ptr()->tags_);

  if (IsCreatedFromSnapshot()) {
    ASSERT(kind != Snapshot::kFull);
    // Write out library URL so that it can be looked up when reading.
    writer->WriteObject(ptr()->url_);
  } else {
    // Write out all non object fields.
    writer->WriteIntptrValue(ptr()->num_imports_);
    writer->WriteIntptrValue(ptr()->num_imported_into_);
    writer->WriteIntptrValue(ptr()->num_anonymous_);
    writer->Write<bool>(ptr()->corelib_imported_);
    writer->Write<int8_t>(ptr()->load_state_);
    // We do not serialize the native resolver over, this needs to be explicitly
    // set after deserialization.
    writer->Write<Dart_NativeEntryResolver>(NULL);
    // We do not write the loaded_scripts_ cache to the snapshot. It gets
    // set to NULL when reading the library from the snapshot, and will
    // be rebuilt lazily.

    // Write out all the object pointer fields.
    SnapshotWriterVisitor visitor(writer);
    visitor.VisitPointers(from(), to());
  }
}


RawLibraryPrefix* LibraryPrefix::ReadFrom(SnapshotReader* reader,
                                          intptr_t object_id,
                                          intptr_t tags,
                                          Snapshot::Kind kind) {
  ASSERT(reader != NULL);
  ASSERT(kind != Snapshot::kMessage && !RawObject::IsCreatedFromSnapshot(tags));

  // Allocate library prefix object.
  LibraryPrefix& prefix = LibraryPrefix::ZoneHandle(
      reader->isolate(), NEW_OBJECT(LibraryPrefix));
  reader->AddBackwardReference(object_id, &prefix);

  // Set the object tags.
  prefix.set_tags(tags);

  // Set all non object fields.
  prefix.raw_ptr()->num_libs_ = reader->ReadIntptrValue();

  // Set all the object fields.
  // TODO(5411462): Need to assert No GC can happen here, even though
  // allocations may happen.
  intptr_t num_flds = (prefix.raw()->to() - prefix.raw()->from());
  for (intptr_t i = 0; i <= num_flds; i++) {
    *(prefix.raw()->from() + i) = reader->ReadObject();
  }

  return prefix.raw();
}


void RawLibraryPrefix::WriteTo(SnapshotWriter* writer,
                               intptr_t object_id,
                               Snapshot::Kind kind) {
  ASSERT(writer != NULL);
  ASSERT(kind != Snapshot::kMessage && !IsCreatedFromSnapshot());

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kLibraryPrefixClass, ptr()->tags_);

  // Write out all non object fields.
  writer->WriteIntptrValue(ptr()->num_libs_);

  // Write out all the object pointer fields.
  SnapshotWriterVisitor visitor(writer);
  visitor.VisitPointers(from(), to());
}


RawCode* Code::ReadFrom(SnapshotReader* reader,
                        intptr_t object_id,
                        intptr_t tags,
                        Snapshot::Kind kind) {
  UNREACHABLE();
  return Code::null();
}


void RawCode::WriteTo(SnapshotWriter* writer,
                      intptr_t object_id,
                      Snapshot::Kind kind) {
  // We have already checked for this and written a NULL object, hence we
  // should not reach here.
  UNREACHABLE();
}


RawInstructions* Instructions::ReadFrom(SnapshotReader* reader,
                                        intptr_t object_id,
                                        intptr_t tags,
                                        Snapshot::Kind kind) {
  UNREACHABLE();
  return Instructions::null();
}


void RawInstructions::WriteTo(SnapshotWriter* writer,
                              intptr_t object_id,
                              Snapshot::Kind kind) {
  UNREACHABLE();
}


RawPcDescriptors* PcDescriptors::ReadFrom(SnapshotReader* reader,
                                          intptr_t object_id,
                                          intptr_t tags,
                                          Snapshot::Kind kind) {
  UNREACHABLE();
  return PcDescriptors::null();
}


void RawPcDescriptors::WriteTo(SnapshotWriter* writer,
                               intptr_t object_id,
                               Snapshot::Kind kind) {
  UNREACHABLE();
}


RawStackmap* Stackmap::ReadFrom(SnapshotReader* reader,
                                intptr_t object_id,
                                intptr_t tags,
                                Snapshot::Kind kind) {
  UNREACHABLE();
  return Stackmap::null();
}


void RawStackmap::WriteTo(SnapshotWriter* writer,
                          intptr_t object_id,
                          Snapshot::Kind kind) {
  UNREACHABLE();
}


RawLocalVarDescriptors* LocalVarDescriptors::ReadFrom(SnapshotReader* reader,
                                                      intptr_t object_id,
                                                      intptr_t tags,
                                                      Snapshot::Kind kind) {
  UNREACHABLE();
  return LocalVarDescriptors::null();
}


void RawLocalVarDescriptors::WriteTo(SnapshotWriter* writer,
                                     intptr_t object_id,
                                     Snapshot::Kind kind) {
  UNREACHABLE();
}


RawExceptionHandlers* ExceptionHandlers::ReadFrom(SnapshotReader* reader,
                                                  intptr_t object_id,
                                                  intptr_t tags,
                                                  Snapshot::Kind kind) {
  UNREACHABLE();
  return ExceptionHandlers::null();
}


void RawExceptionHandlers::WriteTo(SnapshotWriter* writer,
                                   intptr_t object_id,
                                   Snapshot::Kind kind) {
  UNREACHABLE();
}


RawContext* Context::ReadFrom(SnapshotReader* reader,
                              intptr_t object_id,
                              intptr_t tags,
                              Snapshot::Kind kind) {
  ASSERT(reader != NULL);

  // Allocate context object.
  intptr_t num_vars = reader->ReadIntptrValue();
  Context& context = Context::ZoneHandle(reader->isolate(), Context::null());
  if (kind == Snapshot::kFull) {
    context = reader->NewContext(num_vars);
  } else {
    context = Context::New(num_vars);
  }
  reader->AddBackwardReference(object_id, &context);

  // Set the object tags.
  context.set_tags(tags);

  // Set the isolate implicitly.
  context.set_isolate(Isolate::Current());

  // Set all the object fields.
  // TODO(5411462): Need to assert No GC can happen here, even though
  // allocations may happen.
  intptr_t num_flds = (context.raw()->to(num_vars) - context.raw()->from());
  for (intptr_t i = 0; i <= num_flds; i++) {
    *(context.raw()->from() + i) = reader->ReadObject();
  }

  return context.raw();
}


void RawContext::WriteTo(SnapshotWriter* writer,
                         intptr_t object_id,
                         Snapshot::Kind kind) {
  ASSERT(writer != NULL);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kContextClass, ptr()->tags_);

  // Write out num of variables in the context.
  writer->WriteIntptrValue(ptr()->num_variables_);

  // Can't serialize the isolate pointer, we set it implicitly on read.

  // Write out all the object pointer fields.
  SnapshotWriterVisitor visitor(writer);
  visitor.VisitPointers(from(), to(ptr()->num_variables_));
}


RawContextScope* ContextScope::ReadFrom(SnapshotReader* reader,
                                        intptr_t object_id,
                                        intptr_t tags,
                                        Snapshot::Kind kind) {
  ASSERT(reader != NULL);
  ASSERT(kind == Snapshot::kMessage);

  // Allocate context scope object.
  intptr_t num_vars = reader->ReadIntptrValue();
  ContextScope& scope = ContextScope::ZoneHandle(reader->isolate(),
                                                 ContextScope::New(num_vars));
  reader->AddBackwardReference(object_id, &scope);

  // Set the object tags.
  scope.set_tags(tags);

  // Set all the object fields.
  // TODO(5411462): Need to assert No GC can happen here, even though
  // allocations may happen.
  intptr_t num_flds = (scope.raw()->to(num_vars) - scope.raw()->from());
  for (intptr_t i = 0; i <= num_flds; i++) {
    *(scope.raw()->from() + i) = reader->ReadObject();
  }

  return scope.raw();
}


void RawContextScope::WriteTo(SnapshotWriter* writer,
                              intptr_t object_id,
                              Snapshot::Kind kind) {
  ASSERT(writer != NULL);
  ASSERT(kind == Snapshot::kMessage);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(Object::kContextScopeClass, ptr()->tags_);

  // Serialize number of variables.
  writer->WriteIntptrValue(ptr()->num_variables_);

  // Write out all the object pointer fields.
  SnapshotWriterVisitor visitor(writer);
  visitor.VisitPointers(from(), to(ptr()->num_variables_));
}


RawICData* ICData::ReadFrom(SnapshotReader* reader,
                            intptr_t object_id,
                            intptr_t tags,
                            Snapshot::Kind kind) {
  UNREACHABLE();
  return NULL;
}


void RawICData::WriteTo(SnapshotWriter* writer,
                        intptr_t object_id,
                        Snapshot::Kind kind) {
  UNREACHABLE();
}


RawError* Error::ReadFrom(SnapshotReader* reader,
                          intptr_t object_id,
                          intptr_t tags,
                          Snapshot::Kind kind) {
  UNREACHABLE();
  return Error::null();
}


void RawError::WriteTo(SnapshotWriter* writer,
                       intptr_t object_id,
                       Snapshot::Kind kind) {
  UNREACHABLE();
}


RawApiError* ApiError::ReadFrom(SnapshotReader* reader,
                                intptr_t object_id,
                                intptr_t tags,
                                Snapshot::Kind kind) {
  UNIMPLEMENTED();
  return ApiError::null();
}


void RawApiError::WriteTo(SnapshotWriter* writer,
                          intptr_t object_id,
                          Snapshot::Kind kind) {
  UNIMPLEMENTED();
}


RawLanguageError* LanguageError::ReadFrom(SnapshotReader* reader,
                                          intptr_t object_id,
                                          intptr_t tags,
                                          Snapshot::Kind kind) {
  UNIMPLEMENTED();
  return LanguageError::null();
}


void RawLanguageError::WriteTo(SnapshotWriter* writer,
                               intptr_t object_id,
                               Snapshot::Kind kind) {
  UNIMPLEMENTED();
}


RawUnhandledException* UnhandledException::ReadFrom(SnapshotReader* reader,
                                                    intptr_t object_id,
                                                    intptr_t tags,
                                                    Snapshot::Kind kind) {
  UNIMPLEMENTED();
  return UnhandledException::null();
}


void RawUnhandledException::WriteTo(SnapshotWriter* writer,
                                    intptr_t object_id,
                                    Snapshot::Kind kind) {
  UNIMPLEMENTED();
}


RawUnwindError* UnwindError::ReadFrom(SnapshotReader* reader,
                                      intptr_t object_id,
                                      intptr_t tags,
                                      Snapshot::Kind kind) {
  UNIMPLEMENTED();
  return UnwindError::null();
}


void RawUnwindError::WriteTo(SnapshotWriter* writer,
                             intptr_t object_id,
                             Snapshot::Kind kind) {
  UNIMPLEMENTED();
}


RawInstance* Instance::ReadFrom(SnapshotReader* reader,
                                intptr_t object_id,
                                intptr_t tags,
                                Snapshot::Kind kind) {
  UNREACHABLE();
  return Instance::null();
}


void RawInstance::WriteTo(SnapshotWriter* writer,
                          intptr_t object_id,
                          Snapshot::Kind kind) {
  UNREACHABLE();
}


RawMint* Mint::ReadFrom(SnapshotReader* reader,
                        intptr_t object_id,
                        intptr_t tags,
                        Snapshot::Kind kind) {
  ASSERT(reader != NULL);

  // Read the 64 bit value for the object.
  int64_t value = reader->Read<int64_t>();

  // Create a Mint object or get canonical one if it is a canonical constant.
  Mint& mint = Mint::ZoneHandle(reader->isolate(), Mint::null());
  if (kind == Snapshot::kFull) {
    mint = reader->NewMint(value);
  } else {
    if (RawObject::IsCanonical(tags)) {
      mint = Mint::NewCanonical(value);
    } else {
      mint = Mint::New(value, Heap::kNew);
    }
  }
  reader->AddBackwardReference(object_id, &mint);

  // Set the object tags.
  mint.set_tags(tags);

  return mint.raw();
}


void RawMint::WriteTo(SnapshotWriter* writer,
                      intptr_t object_id,
                      Snapshot::Kind kind) {
  ASSERT(writer != NULL);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(ObjectStore::kMintClass, ptr()->tags_);

  // Write out the 64 bit value.
  writer->Write<int64_t>(ptr()->value_);
}


RawBigint* Bigint::ReadFrom(SnapshotReader* reader,
                            intptr_t object_id,
                            intptr_t tags,
                            Snapshot::Kind kind) {
  ASSERT(reader != NULL);

  // Read in the HexCString representation of the bigint.
  intptr_t len = reader->ReadIntptrValue();
  char* str = reinterpret_cast<char*>(ZoneAllocator(len + 1));
  str[len] = '\0';
  for (intptr_t i = 0; i < len; i++) {
    str[i] = reader->Read<uint8_t>();
  }

  // Create a Bigint object from HexCString.
  Bigint& obj = Bigint::ZoneHandle(
      reader->isolate(),
      (kind == Snapshot::kFull) ? reader->NewBigint(str) :
                                  BigintOperations::FromHexCString(str));

  // If it is a canonical constant make it one.
  if ((kind != Snapshot::kFull) && RawObject::IsCanonical(tags)) {
    obj ^= obj.Canonicalize();
  }
  reader->AddBackwardReference(object_id, &obj);

  // Set the object tags.
  obj.set_tags(tags);

  return obj.raw();
}


void RawBigint::WriteTo(SnapshotWriter* writer,
                        intptr_t object_id,
                        Snapshot::Kind kind) {
  ASSERT(writer != NULL);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(ObjectStore::kBigintClass, ptr()->tags_);

  // Write out the bigint value as a HEXCstring.
  intptr_t length = ptr()->signed_length_;
  bool is_negative = false;
  if (length <= 0) {
    length = -length;
    is_negative = true;
  }
  uword data_start = reinterpret_cast<uword>(ptr()) + sizeof(RawBigint);
  const char* str = BigintOperations::ToHexCString(
      length,
      is_negative,
      reinterpret_cast<void*>(data_start),
      &ZoneAllocator);
  bool neg = false;
  if (*str == '-') {
    neg = true;
    str++;
  }
  intptr_t len = strlen(str);
  ASSERT(len > 2 && str[0] == '0' && str[1] == 'x');
  if (neg) {
    writer->WriteIntptrValue(len - 1);  // Include '-' in length.
    writer->Write<uint8_t>('-');
  } else {
    writer->WriteIntptrValue(len - 2);
  }
  for (intptr_t i = 2; i < len; i++) {
    writer->Write<uint8_t>(str[i]);
  }
}


RawDouble* Double::ReadFrom(SnapshotReader* reader,
                            intptr_t object_id,
                            intptr_t tags,
                            Snapshot::Kind kind) {
  ASSERT(reader != NULL);
  // Read the double value for the object.
  double value = reader->Read<double>();

  // Create a Double object or get canonical one if it is a canonical constant.
  Double& dbl = Double::ZoneHandle(reader->isolate(), Double::null());
  if (kind == Snapshot::kFull) {
    dbl = reader->NewDouble(value);
  } else {
    if (RawObject::IsCanonical(tags)) {
      dbl = Double::NewCanonical(value);
    } else {
      dbl = Double::New(value, Heap::kNew);
    }
  }
  reader->AddBackwardReference(object_id, &dbl);

  // Set the object tags.
  dbl.set_tags(tags);

  return dbl.raw();
}


void RawDouble::WriteTo(SnapshotWriter* writer,
                        intptr_t object_id,
                        Snapshot::Kind kind) {
  ASSERT(writer != NULL);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(ObjectStore::kDoubleClass, ptr()->tags_);

  // Write out the double value.
  writer->Write<double>(ptr()->value_);
}


RawString* String::ReadFrom(SnapshotReader* reader,
                            intptr_t object_id,
                            intptr_t tags,
                            Snapshot::Kind kind) {
  UNREACHABLE();  // String is an abstract class.
  return String::null();
}


void RawString::WriteTo(SnapshotWriter* writer,
                        intptr_t object_id,
                        Snapshot::Kind kind) {
  UNREACHABLE();  // String is an abstract class.
}


template<typename HandleType, typename CharacterType>
void String::ReadFromImpl(SnapshotReader* reader,
                          HandleType* str_obj,
                          intptr_t len,
                          intptr_t tags) {
  ASSERT(reader != NULL);
  if (RawObject::IsCanonical(tags)) {
    // Set up canonical string object.
    ASSERT(reader != NULL);
    CharacterType* ptr = reinterpret_cast<CharacterType*>(ZoneAllocator(len));
    for (intptr_t i = 0; i < len; i++) {
      ptr[i] = reader->Read<CharacterType>();
    }
    *str_obj ^= String::NewSymbol(ptr, len);
  } else {
    // Set up the string object.
    *str_obj = HandleType::New(len, Heap::kNew);
    str_obj->set_tags(tags);
    str_obj->SetHash(0);  // Will get computed when needed.
    for (intptr_t i = 0; i < len; i++) {
      *str_obj->CharAddr(i) = reader->Read<CharacterType>();
    }
  }
}


RawOneByteString* OneByteString::ReadFrom(SnapshotReader* reader,
                                          intptr_t object_id,
                                          intptr_t tags,
                                          Snapshot::Kind kind) {
  // Read the length so that we can determine instance size to allocate.
  ASSERT(reader != NULL);
  intptr_t len = reader->ReadSmiValue();
  intptr_t hash = reader->ReadSmiValue();
  OneByteString& str_obj = OneByteString::ZoneHandle(reader->isolate(),
                                                     OneByteString::null());

  if (kind == Snapshot::kFull) {
    ASSERT(reader->isolate()->no_gc_scope_depth() != 0);
    RawOneByteString* obj = reader->NewOneByteString(len);
    str_obj = obj;
    str_obj.set_tags(tags);
    obj->ptr()->hash_ = Smi::New(hash);
    uint8_t* raw_ptr = (len > 0) ? str_obj.CharAddr(0) : NULL;
    for (intptr_t i = 0; i < len; i++) {
      ASSERT(str_obj.CharAddr(i) == raw_ptr);  // Will trigger assertions.
      *raw_ptr = reader->Read<uint8_t>();
      raw_ptr += 1;
    }
    ASSERT((hash == 0) || (String::Hash(str_obj, 0, str_obj.Length()) == hash));
  } else {
    ReadFromImpl<OneByteString, uint8_t>(reader, &str_obj, len, tags);
  }
  reader->AddBackwardReference(object_id, &str_obj);
  return str_obj.raw();
}


RawTwoByteString* TwoByteString::ReadFrom(SnapshotReader* reader,
                                          intptr_t object_id,
                                          intptr_t tags,
                                          Snapshot::Kind kind) {
  // Read the length so that we can determine instance size to allocate.
  ASSERT(reader != NULL);
  intptr_t len = reader->ReadSmiValue();
  intptr_t hash = reader->ReadSmiValue();
  TwoByteString& str_obj = TwoByteString::ZoneHandle(reader->isolate(),
                                                     TwoByteString::null());

  if (kind == Snapshot::kFull) {
    RawTwoByteString* obj = reader->NewTwoByteString(len);
    str_obj = obj;
    str_obj.set_tags(tags);
    obj->ptr()->hash_ = Smi::New(hash);
    uint16_t* raw_ptr = (len > 0)? str_obj.CharAddr(0) : NULL;
    for (intptr_t i = 0; i < len; i++) {
      ASSERT(str_obj.CharAddr(i) == raw_ptr);  // Will trigger assertions.
      *raw_ptr = reader->Read<uint16_t>();
      raw_ptr += 1;
    }
    ASSERT(String::Hash(str_obj, 0, str_obj.Length()) == hash);
  } else {
    ReadFromImpl<TwoByteString, uint16_t>(reader, &str_obj, len, tags);
  }
  reader->AddBackwardReference(object_id, &str_obj);
  return str_obj.raw();
}


RawFourByteString* FourByteString::ReadFrom(SnapshotReader* reader,
                                            intptr_t object_id,
                                            intptr_t tags,
                                            Snapshot::Kind kind) {
  // Read the length so that we can determine instance size to allocate.
  ASSERT(reader != NULL);
  intptr_t len = reader->ReadSmiValue();
  intptr_t hash = reader->ReadSmiValue();
  FourByteString& str_obj = FourByteString::ZoneHandle(reader->isolate(),
                                                       FourByteString::null());

  if (kind == Snapshot::kFull) {
    RawFourByteString* obj = reader->NewFourByteString(len);
    str_obj = obj;
    str_obj.set_tags(tags);
    obj->ptr()->hash_ = Smi::New(hash);
    uint32_t* raw_ptr = (len > 0)? str_obj.CharAddr(0) : NULL;
    for (intptr_t i = 0; i < len; i++) {
      ASSERT(str_obj.CharAddr(i) == raw_ptr);  // Will trigger assertions.
      *raw_ptr = reader->Read<uint32_t>();
      raw_ptr += 1;
    }
    ASSERT(String::Hash(str_obj, 0, str_obj.Length()) == hash);
  } else {
    ReadFromImpl<FourByteString, uint32_t>(reader, &str_obj, len, tags);
  }
  reader->AddBackwardReference(object_id, &str_obj);
  return str_obj.raw();
}


template<typename T>
static void StringWriteTo(SnapshotWriter* writer,
                          intptr_t object_id,
                          Snapshot::Kind kind,
                          intptr_t class_id,
                          intptr_t tags,
                          RawSmi* length,
                          RawSmi* hash,
                          T* data) {
  ASSERT(writer != NULL);
  intptr_t len = Smi::Value(length);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(class_id, tags);

  // Write out the length field.
  writer->Write<RawObject*>(length);

  // Write out the hash field.
  writer->Write<RawObject*>(hash);

  // Write out the string.
  for (intptr_t i = 0; i < len; i++) {
    writer->Write(data[i]);
  }
}


void RawOneByteString::WriteTo(SnapshotWriter* writer,
                               intptr_t object_id,
                               Snapshot::Kind kind) {
  StringWriteTo(writer,
                object_id,
                kind,
                ObjectStore::kOneByteStringClass,
                ptr()->tags_,
                ptr()->length_,
                ptr()->hash_,
                ptr()->data_);
}


void RawTwoByteString::WriteTo(SnapshotWriter* writer,
                               intptr_t object_id,
                               Snapshot::Kind kind) {
  StringWriteTo(writer,
                object_id,
                kind,
                ObjectStore::kTwoByteStringClass,
                ptr()->tags_,
                ptr()->length_,
                ptr()->hash_,
                ptr()->data_);
}


void RawFourByteString::WriteTo(SnapshotWriter* writer,
                                intptr_t object_id,
                                Snapshot::Kind kind) {
  StringWriteTo(writer,
                object_id,
                kind,
                ObjectStore::kFourByteStringClass,
                ptr()->tags_,
                ptr()->length_,
                ptr()->hash_,
                ptr()->data_);
}


RawExternalOneByteString* ExternalOneByteString::ReadFrom(
    SnapshotReader* reader,
    intptr_t object_id,
    intptr_t tags,
    Snapshot::Kind kind) {
  UNREACHABLE();
  return ExternalOneByteString::null();
}


RawExternalTwoByteString* ExternalTwoByteString::ReadFrom(
    SnapshotReader* reader,
    intptr_t object_id,
    intptr_t tags,
    Snapshot::Kind kind) {
  UNREACHABLE();
  return ExternalTwoByteString::null();
}


RawExternalFourByteString* ExternalFourByteString::ReadFrom(
    SnapshotReader* reader,
    intptr_t object_id,
    intptr_t tags,
    Snapshot::Kind kind) {
  UNREACHABLE();
  return ExternalFourByteString::null();
}


void RawExternalOneByteString::WriteTo(SnapshotWriter* writer,
                                       intptr_t object_id,
                                       Snapshot::Kind kind) {
  // Serialize as a non-external one byte string.
  StringWriteTo(writer,
                object_id,
                kind,
                ObjectStore::kOneByteStringClass,
                ptr()->tags_,
                ptr()->length_,
                ptr()->hash_,
                ptr()->external_data_->data());
}


void RawExternalTwoByteString::WriteTo(SnapshotWriter* writer,
                                       intptr_t object_id,
                                       Snapshot::Kind kind) {
  // Serialize as a non-external two byte string.
  StringWriteTo(writer,
                object_id,
                kind,
                ObjectStore::kTwoByteStringClass,
                ptr()->tags_,
                ptr()->length_,
                ptr()->hash_,
                ptr()->external_data_->data());
}


void RawExternalFourByteString::WriteTo(SnapshotWriter* writer,
                                        intptr_t object_id,
                                        Snapshot::Kind kind) {
  // Serialize as a non-external four byte string.
  StringWriteTo(writer,
                object_id,
                kind,
                ObjectStore::kFourByteStringClass,
                ptr()->tags_,
                ptr()->length_,
                ptr()->hash_,
                ptr()->external_data_->data());
}


RawBool* Bool::ReadFrom(SnapshotReader* reader,
                        intptr_t object_id,
                        intptr_t tags,
                        Snapshot::Kind kind) {
  UNREACHABLE();
  return Bool::null();
}


void RawBool::WriteTo(SnapshotWriter* writer,
                      intptr_t object_id,
                      Snapshot::Kind kind) {
  UNREACHABLE();
}


static void ArrayReadFrom(SnapshotReader* reader,
                          const Array& result,
                          intptr_t len,
                          intptr_t tags) {
  ASSERT(reader != NULL);

  // Set the object tags.
  result.set_tags(tags);

  // Setup the object fields.
  *reader->TypeArgumentsHandle() ^= reader->ReadObject();
  result.SetTypeArguments(*reader->TypeArgumentsHandle());

  for (intptr_t i = 0; i < len; i++) {
    *reader->ObjectHandle() = reader->ReadObject();
    result.SetAt(i, *reader->ObjectHandle());
  }
}


RawArray* Array::ReadFrom(SnapshotReader* reader,
                          intptr_t object_id,
                          intptr_t tags,
                          Snapshot::Kind kind) {
  ASSERT(reader != NULL);

  // Read the length so that we can determine instance size to allocate.
  intptr_t len = reader->ReadSmiValue();
  Array& array = Array::ZoneHandle(reader->isolate(),
                                   NEW_OBJECT_WITH_LEN(Array, len));
  reader->AddBackwardReference(object_id, &array);
  ArrayReadFrom(reader, array, len, tags);
  return array.raw();
}


RawImmutableArray* ImmutableArray::ReadFrom(SnapshotReader* reader,
                                            intptr_t object_id,
                                            intptr_t tags,
                                            Snapshot::Kind kind) {
  ASSERT(reader != NULL);

  // Read the length so that we can determine instance size to allocate.
  intptr_t len = reader->ReadSmiValue();
  ImmutableArray& array = ImmutableArray::ZoneHandle(
      reader->isolate(), NEW_OBJECT_WITH_LEN(ImmutableArray, len));
  reader->AddBackwardReference(object_id, &array);
  ArrayReadFrom(reader, array, len, tags);
  return array.raw();
}


static void ArrayWriteTo(SnapshotWriter* writer,
                         intptr_t object_id,
                         Snapshot::Kind kind,
                         intptr_t array_kind,
                         intptr_t tags,
                         RawSmi* length,
                         RawAbstractTypeArguments* type_arguments,
                         RawObject* data[]) {
  ASSERT(writer != NULL);
  intptr_t len = Smi::Value(length);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(array_kind, tags);

  // Write out the length field.
  writer->Write<RawObject*>(length);

  // Write out the type arguments.
  writer->WriteObject(type_arguments);

  // Write out the individual objects.
  for (intptr_t i = 0; i < len; i++) {
    writer->WriteObject(data[i]);
  }
}


void RawArray::WriteTo(SnapshotWriter* writer,
                       intptr_t object_id,
                       Snapshot::Kind kind) {
  ArrayWriteTo(writer,
               object_id,
               kind,
               ObjectStore::kArrayClass,
               ptr()->tags_,
               ptr()->length_,
               ptr()->type_arguments_,
               ptr()->data());
}


void RawImmutableArray::WriteTo(SnapshotWriter* writer,
                                intptr_t object_id,
                                Snapshot::Kind kind) {
  ArrayWriteTo(writer,
               object_id,
               kind,
               ObjectStore::kImmutableArrayClass,
               ptr()->tags_,
               ptr()->length_,
               ptr()->type_arguments_,
               ptr()->data());
}


RawGrowableObjectArray* GrowableObjectArray::ReadFrom(SnapshotReader* reader,
                                                      intptr_t object_id,
                                                      intptr_t tags,
                                                      Snapshot::Kind kind) {
  ASSERT(reader != NULL);

  // Read the length so that we can determine instance size to allocate.
  GrowableObjectArray& array = GrowableObjectArray::ZoneHandle(
      reader->isolate(), reader->NewGrowableObjectArray());
  reader->AddBackwardReference(object_id, &array);
  intptr_t length = reader->ReadSmiValue();
  array.SetLength(length);
  Array& contents = Array::Handle();
  contents ^= reader->ReadObject();
  array.SetData(contents);
  return array.raw();
}


void RawGrowableObjectArray::WriteTo(SnapshotWriter* writer,
                                     intptr_t object_id,
                                     Snapshot::Kind kind) {
  ASSERT(writer != NULL);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(ObjectStore::kGrowableObjectArrayClass,
                            ptr()->tags_);

  // Write out the used length field.
  writer->Write<RawObject*>(ptr()->length_);

  // Write out the Array object.
  writer->WriteObject(ptr()->data_);
}


RawByteArray* ByteArray::ReadFrom(SnapshotReader* reader,
                                  intptr_t object_id,
                                  intptr_t tags,
                                  Snapshot::Kind kind) {
  UNREACHABLE();  // ByteArray is an abstract class.
  return ByteArray::null();
}


RawInternalByteArray* InternalByteArray::ReadFrom(SnapshotReader* reader,
                                                  intptr_t object_id,
                                                  intptr_t tags,
                                                  Snapshot::Kind kind) {
  ASSERT(reader != NULL);

  // Read the length so that we can determine instance size to allocate.
  intptr_t len = reader->ReadSmiValue();
  Heap::Space space = (kind == Snapshot::kFull) ? Heap::kOld : Heap::kNew;
  InternalByteArray& result =
      InternalByteArray::ZoneHandle(reader->isolate(),
                                    InternalByteArray::New(len, space));
  reader->AddBackwardReference(object_id, &result);

  // Set the object tags.
  result.set_tags(tags);

  // Setup the array elements.
  for (intptr_t i = 0; i < len; i++) {
    result.SetAt<uint8_t>(i, reader->Read<uint8_t>());
  }
  return result.raw();
}


RawExternalByteArray* ExternalByteArray::ReadFrom(SnapshotReader* reader,
                                                  intptr_t object_id,
                                                  intptr_t tags,
                                                  Snapshot::Kind kind) {
  UNREACHABLE();
  return ExternalByteArray::null();
}


static void ByteArrayWriteTo(SnapshotWriter* writer,
                             intptr_t object_id,
                             Snapshot::Kind kind,
                             intptr_t byte_array_kind,
                             intptr_t tags,
                             RawSmi* length,
                             uint8_t* data) {
  ASSERT(writer != NULL);
  intptr_t len = Smi::Value(length);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(byte_array_kind, tags);

  // Write out the length field.
  writer->Write<RawObject*>(length);

  // Write out the array elements.
  for (intptr_t i = 0; i < len; i++) {
    writer->Write(data[i]);
  }
}


void RawByteArray::WriteTo(SnapshotWriter* writer,
                           intptr_t object_id,
                           Snapshot::Kind kind) {
  UNREACHABLE();  // ByteArray is an abstract class
}


void RawInternalByteArray::WriteTo(SnapshotWriter* writer,
                                   intptr_t object_id,
                                   Snapshot::Kind kind) {
  ByteArrayWriteTo(writer,
                   object_id,
                   kind,
                   ObjectStore::kInternalByteArrayClass,
                   ptr()->tags_,
                   ptr()->length_,
                   ptr()->data());
}


void RawExternalByteArray::WriteTo(SnapshotWriter* writer,
                                   intptr_t object_id,
                                   Snapshot::Kind kind) {
  // Serialize as an internal byte array.
  ByteArrayWriteTo(writer,
                   object_id,
                   kind,
                   ObjectStore::kInternalByteArrayClass,
                   ptr()->tags_,
                   ptr()->length_,
                   ptr()->external_data_->data());
}


RawClosure* Closure::ReadFrom(SnapshotReader* reader,
                              intptr_t object_id,
                              intptr_t tags,
                              Snapshot::Kind kind) {
  UNIMPLEMENTED();
  return Closure::null();
}


void RawClosure::WriteTo(SnapshotWriter* writer,
                         intptr_t object_id,
                         Snapshot::Kind kind) {
  UNIMPLEMENTED();
}


RawStacktrace* Stacktrace::ReadFrom(SnapshotReader* reader,
                                    intptr_t object_id,
                                    intptr_t tags,
                                    Snapshot::Kind kind) {
  UNIMPLEMENTED();
  return Stacktrace::null();
}


void RawStacktrace::WriteTo(SnapshotWriter* writer,
                            intptr_t object_id,
                            Snapshot::Kind kind) {
  UNIMPLEMENTED();
}


RawJSRegExp* JSRegExp::ReadFrom(SnapshotReader* reader,
                                intptr_t object_id,
                                intptr_t tags,
                                Snapshot::Kind kind) {
  ASSERT(reader != NULL);
  ASSERT(kind == Snapshot::kMessage);

  // Read the length so that we can determine instance size to allocate.
  intptr_t len = reader->ReadSmiValue();

  // Allocate JSRegExp object.
  JSRegExp& regex = JSRegExp::ZoneHandle(
      reader->isolate(),
      JSRegExp::New(len, (kind == Snapshot::kFull) ? Heap::kOld : Heap::kNew));
  reader->AddBackwardReference(object_id, &regex);

  // Set the object tags.
  regex.set_tags(tags);

  // Read and Set all the other fields.
  regex.raw_ptr()->num_bracket_expressions_ = reader->ReadAsSmi();
  *reader->StringHandle() ^= reader->ReadObject();
  regex.raw_ptr()->pattern_ = (*reader->StringHandle()).raw();
  regex.raw_ptr()->type_ = reader->ReadIntptrValue();
  regex.raw_ptr()->flags_ = reader->ReadIntptrValue();

  // TODO(5411462): Need to implement a way of recompiling the regex.

  return regex.raw();
}


void RawJSRegExp::WriteTo(SnapshotWriter* writer,
                          intptr_t object_id,
                          Snapshot::Kind kind) {
  ASSERT(writer != NULL);
  ASSERT(kind == Snapshot::kMessage);

  // Write out the serialization header value for this object.
  writer->WriteSerializationMarker(kInlined, object_id);

  // Write out the class and tags information.
  writer->WriteObjectHeader(ObjectStore::kJSRegExpClass, ptr()->tags_);

  // Write out the data length field.
  writer->Write<RawObject*>(ptr()->data_length_);

  // Write out all the other fields.
  writer->Write<RawObject*>(ptr()->num_bracket_expressions_);
  writer->WriteObject(ptr()->pattern_);
  writer->WriteIntptrValue(ptr()->type_);
  writer->WriteIntptrValue(ptr()->flags_);

  // Do not write out the data part which is native.
}


}  // namespace dart

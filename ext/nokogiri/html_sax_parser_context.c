#include <html_sax_parser_context.h>

VALUE cNokogiriHtmlSaxParserContext ;

static void deallocate(xmlParserCtxtPtr ctxt)
{
  NOKOGIRI_DEBUG_START(handler);

  ctxt->sax = NULL;

  htmlFreeParserCtxt(ctxt);

  NOKOGIRI_DEBUG_END(handler);
}

static VALUE parse_memory(VALUE klass, SEL sel, VALUE data, VALUE encoding)
{
  if(NIL_P(data)) rb_raise(rb_eArgError, "data cannot be nil");
  if(!(int)RSTRING_LEN(data))
    rb_raise(rb_eRuntimeError, "data cannot be empty");

  htmlParserCtxtPtr ctxt = htmlCreateMemoryParserCtxt(
      StringValuePtr(data),
      (int)RSTRING_LEN(data)
  );

  return Data_Wrap_Struct(klass, NULL, deallocate, ctxt);
}

static VALUE parse_file(VALUE klass, SEL sel, VALUE filename, VALUE encoding)
{
  htmlParserCtxtPtr ctxt = htmlCreateFileParserCtxt(
      StringValuePtr(filename),
      StringValuePtr(encoding)
  );
  return Data_Wrap_Struct(klass, NULL, deallocate, ctxt);
}

static VALUE parse_with(VALUE self, SEL sel, VALUE sax_handler)
{
  if(!rb_obj_is_kind_of(sax_handler, cNokogiriXmlSaxParser))
    rb_raise(rb_eArgError, "argument must be a Nokogiri::XML::SAX::Parser");

  htmlParserCtxtPtr ctxt;
  Data_Get_Struct(self, htmlParserCtxt, ctxt);

  htmlSAXHandlerPtr sax;
  Data_Get_Struct(sax_handler, htmlSAXHandler, sax);

  // Free the sax handler since we'll assign our own
  if(ctxt->sax && ctxt->sax != (xmlSAXHandlerPtr)&xmlDefaultSAXHandler)
    xmlFree(ctxt->sax);

  ctxt->sax = sax;
  ctxt->userData = (void *)NOKOGIRI_SAX_TUPLE_NEW(ctxt, sax_handler);

  htmlParseDocument(ctxt);

  if(NULL != ctxt->myDoc) xmlFreeDoc(ctxt->myDoc);

  NOKOGIRI_SAX_TUPLE_DESTROY(ctxt->userData);
}

void init_html_sax_parser_context()
{
  VALUE nokogiri  = rb_define_module("Nokogiri");
  VALUE xml       = rb_define_module_under(nokogiri, "XML");
  VALUE html      = rb_define_module_under(nokogiri, "HTML");
  VALUE sax       = rb_define_module_under(xml, "SAX");
  VALUE hsax      = rb_define_module_under(html, "SAX");
  VALUE pc        = rb_define_class_under(sax, "ParserContext", rb_cObject);
  VALUE klass     = rb_define_class_under(hsax, "ParserContext", pc);

  cNokogiriHtmlSaxParserContext = klass;

  rb_objc_define_method(*(VALUE *)klass, "memory", parse_memory, 2);
  rb_objc_define_method(*(VALUE *)klass, "file", parse_file, 2);

  rb_objc_define_method(klass, "parse_with", parse_with, 1);
}
